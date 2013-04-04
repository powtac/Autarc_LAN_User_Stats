#include <SPI.h>
#include <Ethernet.h> 
#include "ICMPPing.h"
#include "IPHelper.h"
#include "Array.h"


// Shield and network configuration
// WireShark Filter: eth.addr[0:3]==90:A2:DA
boolean useDhcp                    = false; // Using DHCP? If no please set ip_shield, gateway and subnet below

// Heidelberg
static uint8_t mac_shield[6]  = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F };
IPAdress ip_shield            = {{ 10, 0, 1, 13 }, ""};    
IPAdress gateway              = {{ 10, 0, 1, 1 }, ""};
IPAdress subnet               = {{ 255, 255, 0, 0 }, ""};

// IP configuration of known IPs
IPAdress ip_known_device      = {{ 10, 0, 1, 2  }, ""};
IPAdress ip_scan_start        = {{ 10, 0, 1, 0 }, ""};
IPAdress ip_scan_end          = {{ 10, 0, 1, 255 }, ""};

/* 
// Esslingen
static uint8_t mac_shield[6]  = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F };
IPAdress ip_shield              = {{ 192, 168, 1, 30 }, ""};    
IPAdress gateway                = {{ 192, 168, 1, 1 }, ""};
IPAdress subnet                 = {{ 255, 255, 0, 0 }, ""};
    
// IP configuration of known IPs
IPAdress ip_known_device      = {{ 192, 168, 1, 2  }, ""};
IPAdress ip_scan_start        = {{ 192, 168, 1, 0 }, ""};
IPAdress ip_scan_end          = {{ 192, 168, 1, 255 }, ""};
*/

IPAdress ip_to_scan           = ip_scan_start; 

// Arrays for found and possible devices
IPAdresses ip_found_devices;
IPAdresses ip_possible_devices;

// Counter for the loop
int i                           = 0;
int k                           = 0;

// Ping library configuration
SOCKET pingSocket               = 0;
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
   


  // Testing known address
  ping(1, ip_known_device.ipadress, buffer);
  Serial.println(buffer);
  readable_ip(ip_known_device.ipadress, "Testing known address %s\n"); 
  
  
  // Array stuff
  initArray(&ip_found_devices, 0);
  initArray(&ip_possible_devices, 0);
  
  
  // TODO get actual IP range from DHCP
  // TODO fix bug here somewhere
  for (int c = 0; c < 250; ++c) {
    ip_scan_start.ipadress[3] = (byte)ip_scan_start.ipadress[3] + 1;
    if (memcmp(ip_scan_start.ipadress, ip_shield.ipadress,       sizeof(ip_scan_start.ipadress)) != 0 || 
        memcmp(ip_scan_start.ipadress, ip_known_device.ipadress, sizeof(ip_scan_start.ipadress)) != 0)
      insertArray(&ip_possible_devices, ip_scan_start);
  }
  
  
  Serial.println("\nStarting loop trough IP range\n");
}



void loop() {

  if (i < ip_possible_devices.used) {
    
    ping(1, ip_possible_devices.array[i].ipadress, buffer);
    String ping_result = buffer;
    
    if (ping_result.indexOf("Timed Out") == -1) {
      // We found a device!
      ip_possible_devices.array[k].mac = ping_result.substring(0, 18);
      insertArray(&ip_found_devices, ip_possible_devices.array[i]);  
      // remove_element(&ip_possible_devices, i);
      
      readable_ip(ip_found_devices.array[ip_found_devices.used-1].ipadress, "Device found on: %s\n");
      readable_mac(ip_possible_devices.array[k].mac, "The MAC address of the found device %s\n");
    } else {
      // It's not responding, next one
      readable_ip(ip_possible_devices.array[i].ipadress, "No (pingable) device on IP: %s\n");
    }
    
  } else {
    
    if (k == 0) Serial.println("Check found IPs");
    if (k < ip_found_devices.used) {
      
        // Todo check for more known IPs (gateway, ...)
        ping(1, ip_found_devices.array[k].ipadress, buffer);
        String ping_result = buffer;
        
        if (ping_result.indexOf("Timed Out") != -1) {
          // We couldn't find IP Anymore, add it to possible ips and remove it from found ips
          insertArray(&ip_possible_devices, ip_found_devices.array[k]);
          remove_element(&ip_found_devices, k);
          readable_ip(ip_possible_devices.array[ip_possible_devices.used-1].ipadress, "Device lost on: %s");
        } else {
          readable_ip(ip_found_devices.array[k].ipadress, "Device still online: %s");
        }
        k++;
    } else {
      // Hier ggf. ip_found_devices durchlaufen und mac verschicken
      
      Serial.println("\nRestart from .0");
      i = 0;
      k = 0;
    }
    
  }
  i++;  // Der läuft einmal zu viel durch oben..
}





