#include <SPI.h>
#include <Ethernet.h> 
#include "ICMPPing.h"
#include "IPHelper.h"
#include "Array.h"


// Shield and network configuration
// WireShark Filter: eth.addr[0:3]==90:A2:DA
bool useDhcp = false; // Using DHCP? If no please set ip_shield, gateway and subnet below
static uint8_t mac_shield[6]  = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F };
IPAdress ip_shield              = {{ 192, 168, 1, 30 }, ""};    
IPAdress gateway                = {{ 192, 168, 1, 1 }, ""};
IPAdress subnet                 = {{ 255, 255, 0, 0 }, ""};

// IP configuration of known IPs
IPAdress ip_known_client       = {{ 192, 168, 1, 2  }, ""};
IPAdress ip_scan_start         = {{ 192, 168, 1, 0 }, ""};
IPAdress ip_scan_end           = {{ 192, 168, 1, 255 }, ""};
IPAdress ip_to_scan            = ip_scan_start;

// Arrays for found and possible clients
IPAdresses ip_found_clients;
IPAdresses ip_possible_clients;

// Counter for the loop
int i                         = 0;
int k                         = 0;

// Ping library configuration
SOCKET pingSocket             = 0;
char buffer [256];

// Global namespace to use it in setup() and loop()
ICMPPing ping(pingSocket);

void setup() {
  Serial.begin(9600);
  
  // Setup Start
  Serial.println("Try to get IP address from network...");
  readable_mac(mac_shield, " MAC address of shield: %s\n");
  
  // Setup when no IP is known
  if (useDhcp == false) {
    Ethernet.begin(mac_shield, ip_shield.ipadress, gateway.ipadress, subnet.ipadress);
  } else {
    if (Ethernet.begin(mac_shield) == 0) {
      Serial.println("DHCP failed, no automatic IP address assigned!");
      Serial.println("Time for waiting for IP address: "+millis());
      Serial.println("Trying to set manual IP address.");
      Ethernet.begin(mac_shield, ip_shield.ipadress, gateway.ipadress, subnet.ipadress);
    }
  }

  Serial.println(" Address assigned?");
  Serial.print(" ");
  Serial.println(Ethernet.localIP());
  Serial.println(" Setup complete\n");
  // Setup End
  
  initArray(&ip_found_clients, 0);
  initArray(&ip_possible_clients, 0);
  
  
  // Testing known address
  ping(1, ip_known_client.ipadress, buffer);
  //Serial.println(buffer);
  readable_ip(ip_known_client.ipadress, "Testing known address %s"); 


  // Pinging first address of address range
  ping(1, ip_scan_start.ipadress, buffer);
  //Serial.print(" ");
  //Serial.println(buffer);
  readable_ip(ip_known_client.ipadress, "Testing first address %s of address range"); 
  
  // Adding Known Ip to known IPs
  insertArray(&ip_found_clients, ip_known_client);  
  
  for (int c = 0; c < 250; ++c) {
    ip_scan_start.ipadress[3] = (byte)ip_scan_start.ipadress[3] + 1;
    if (memcmp(ip_scan_start.ipadress, ip_shield.ipadress, sizeof(ip_scan_start.ipadress)) != 0 || memcmp(ip_scan_start.ipadress, ip_known_client.ipadress, sizeof(ip_scan_start.ipadress)) != 0)
      insertArray(&ip_possible_clients, ip_scan_start);
  }
  
  Serial.println("\n");
  Serial.println("Starting loop trough IP range");
  
}



void loop() {
  Serial.println("\n");
  if (i < ip_possible_clients.used) {
    ping(1, ip_possible_clients.array[i].ipadress, buffer);
    String ping_result = buffer;
    if (ping_result.indexOf("Timed Out") == -1) {
      // We found a device!
      ip_possible_clients.array[k].mac = ping_result.substring(0, 18);
      Serial.println(ip_possible_clients.array[k].mac);
      insertArray(&ip_found_clients, ip_possible_clients.array[i]);  
      remove_element(&ip_possible_clients, i);
      readable_ip(ip_found_clients.array[ip_found_clients.used-1].ipadress, "Device found on: %s");
    } else {
      // It's not responding, next one
      readable_ip(ip_possible_clients.array[i].ipadress, "No (pingable) device on IP: %s");
    }
    i++;
  } else {
    i++;  // Der läuft einmal zu viel durch oben..
    if (k == 0) Serial.println("Check found IPs");
    if (k < ip_found_clients.used) {
        // Todo check for more known IPs (gateway, ...)
        ping(1, ip_found_clients.array[k].ipadress, buffer);
        String ping_result = buffer;
        if (ping_result.indexOf("Timed Out") != -1) {
          // We couldn't find IP Anymore, add it to possible ips and remove it from found ips
          insertArray(&ip_possible_clients, ip_found_clients.array[k]);
          remove_element(&ip_found_clients, k);
          readable_ip(ip_possible_clients.array[ip_possible_clients.used-1].ipadress, "Device lost on: %s");
        } else {
          readable_ip(ip_found_clients.array[k].ipadress, "Device still online: %s");
        }
        k++;
    } else {
      // Hier ip_found_clients durchlaufen und mac verschicken
      
      Serial.println("\nRestart from .0");
      i = 0;
      k = 0;
    }
  }
}





