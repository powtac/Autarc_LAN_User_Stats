#include <SPI.h>
#include <Ethernet.h> 
// See https://github.com/BlakeFoster/Arduino-Ping/
#include "ICMPPing.h"
#include "IPHelper.h"
#include "Array.h"

// Shield and network configuration
// WireShark Filter: eth.addr[0:3]==90:A2:DA
boolean useDhcp                = true; // Using DHCP? If no please set ip_shield, gateway and subnet below
boolean scanKnownIPOnStart     = false;


// Heidelberg
static uint8_t mac_shield[6]   = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F };
byte ip_shield[4]              = { 10, 0, 1, 13 };    
byte gateway[4]                = { 10, 0, 1, 1 };
byte subnet[4]                 = { 255, 255, 0, 0 };

// IP configuration of known IPs
byte ip_known_device[4]        = { 10, 0, 1, 2  };
byte ip_scan_start[4]          = { 10, 0, 1, 0 };
byte ip_scan_end[4]            = { 10, 0, 1, 255 };

/*
// Esslingen
static uint8_t mac_shield[6]   = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F };
byte ip_shield[4]              = { 192, 168, 1, 30 };    
byte gateway[4]                = { 192, 168, 1, 1 };
byte subnet[4]                 = { 255, 255, 0, 0 };
    
// IP configuration of known IPs
byte ip_known_device[4]        = { 192, 168, 1, 2  };
byte ip_scan_start[4]          = { 192, 168, 1, 0 };
byte ip_scan_end[4]            = { 192, 168, 1, 255 };
*/

byte* ip_to_scan               = ip_scan_start; 

// Arrays for found and possible devices
IPAdresses ip_found_devices;
IPAdresses ip_possible_devices;

// Counter for the loop
int i                          = 0;
int k                          = 0;

// Ping library configuration
SOCKET pingSocket              = 0;

// HTTP server in the internet
// EthernetClient client;
// IPAddress server(64, 233, 187, 99);
// IPAddress server(85,10,211,16); // kolchose.org

// Global namespace to use it in setup() and loop()
ICMPPing ping(pingSocket, (uint16_t)random(0, 255));




void setup() {
  delay(1000);
  Serial.begin(115200);

  // Setup Start
  Serial.println("Try to get IP address from network...");
  readable_mac(mac_shield, " MAC address of shield: %s\n");
  
  // Setup when no IP is known
  if (useDhcp == false) {
    Ethernet.begin(mac_shield, ip_shield, gateway, subnet);
  } else {
    if (Ethernet.begin(mac_shield) == 0) {
      Serial.println("DHCP failed, no automatic IP address assigned!");
      Serial.println("Time for waiting for IP address: "+millis());
      Serial.println("Trying to set manual IP address.");
      Ethernet.begin(mac_shield, ip_shield, gateway, subnet);
    }
  }

  Serial.println(" Address assigned?");
  Serial.print(" ");
  Serial.println(Ethernet.localIP());
  Serial.println(" Setup complete\n");
  // Setup End
   


  // Testing known address
  if (scanKnownIPOnStart) {
    ICMPEchoReply echoReply = ping(ip_known_device, 4);
    readable_ip(ip_known_device, "Testing known address %s\n");
  }
    
  // Array stuff
  initArray(&ip_found_devices, 0);
  initArray(&ip_possible_devices, 0);
  
  
  // TODO get actual IP range from DHCP
  // TODO fix bug here somewhere
  for (int c = 0; c < 10; ++c) {
    ip_scan_start[3] = (byte)ip_scan_start[3] + 1;
    // Code um IPs auszulassen wird später wieder eingeführt
    // if (memcmp(ip_scan_start, ip_shield,       sizeof(ip_scan_start)) != 0 || // Ip
    //  memcmp(ip_scan_start, ip_known_device, sizeof(ip_scan_start)) != 0) {
    IPAdress xy = {ip_scan_start[3]};
    insertArray(&ip_possible_devices, xy);
    //}
  }
  
  /*
  delay(1000);
  EthernetClient client;
  IPAddress server(85, 10, 211, 16); // kolchose.org 
  
  int ret = client.connect("kolchose.org", 80);
  if (ret == 1) {
    Serial.println("Connected to HTTP Server");
    
    // Make a HTTP request:
    client.print("GET /autarc_lan_user_stats/?");
    client.print("IPs%20gefunden");
    client.println(" HTTP/1.0");
    
    client.println("Host: kolchose.org");           // Important! TODO check if this is required and dynamically asignable
    client.println("User-Agent: Autarc_LAN_User_Stats"); // Important!
    client.println();                               // Important!
  } 
  
  Serial.println(client.status());
  client.stop();
  */
  
  int   nr_of_ips_to_scan = ip_scan_end[3] - ip_scan_start[3];
  byte* ips_to_scan[nr_of_ips_to_scan];
 
  for (int i = 0; i < nr_of_ips_to_scan; i++) {
    byte tmp[4] = { ip_scan_start[0], ip_scan_start[1], ip_scan_start[2], ip_scan_start[3] + i};
    ips_to_scan[i] = tmp;
    
    // Debug
    // Serial.println(byte_to_readable_ip_string(tmp));  
  }
  
  
  // TODO into one line
  Serial.print("\nStarting loop trough IP range ");
  Serial.print(byte_to_readable_ip_string(ip_scan_start));
  Serial.print(" - ");
  Serial.print(byte_to_readable_ip_string(ip_scan_end));
  Serial.print("...\n");
}

void loop() {
  byte currIp[4];
  for(int b = 0; b < 4; b++) { currIp[b]  = ip_scan_start[b]; }
  if (i < ip_possible_devices.used) {
    
    currIp[3] = ip_possible_devices.array[i].ipadress; 
    
    ICMPEchoReply echoReply = ping(currIp, 4);
    
    if (echoReply.status == SUCCESS) {
      // We found a device!
      // ip_possible_devices.array[k].mac = ping_result.substring(0, 18);
      insertArray(&ip_found_devices, ip_possible_devices.array[i]);  
      remove_element(&ip_possible_devices, i);
      i--;
      readable_ip(currIp, "Device found on: %s\n");
      // readable_mac(ip_possible_devices.array[k].mac, "The MAC address of the found device %s\n");
    } else {
      // It's not responding, next one
      readable_ip(currIp, "No (pingable) device on IP: %s\n");
    }
    i++;  // Der läuft einmal zu viel durch oben.
  } else {
    
    if (k == 0) Serial.println("\nCheck found IPs...");
    if (k < ip_found_devices.used) {
      
        // Todo check for more known IPs (gateway, ...)
        currIp[3] = ip_found_devices.array[k].ipadress; 

        ICMPEchoReply echoReply = ping(currIp, 4);

        if (echoReply.status == SUCCESS) {
          // We couldn't find IP Anymore, add it to possible ips and remove it from found ips
          insertArray(&ip_possible_devices, ip_found_devices.array[k]);
          remove_element(&ip_found_devices, k);
          k--;
          readable_ip(currIp, "Device lost on: %s\n");
        } else {
          readable_ip(currIp, "Device still online: %s\n");
        }
        k++;
    } else {
      // Hier ggf. ip_found_devices durchlaufen und mac verschicken
      
      // send_info_to_server("IPs gefunden");
      
      Serial.println("\nRestart loop from .0");
      i = 0;
      k = 0;
    }
  }
  
}







