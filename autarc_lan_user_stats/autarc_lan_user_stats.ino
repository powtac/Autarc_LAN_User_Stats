
#include <SPI.h>
#include <Ethernet.h> 
#include "ICMPPing.h"
#include "IPHelper.h"
#include "Array.h"


// Shield and network configuration
// WireShark Filter: eth.addr[0:3]==90:A2:DA
static uint8_t mac_shield[6]  = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F };
IPAdress ip_shield              = {{ 192, 168, 1, 30 }};    
IPAdress gateway                = {{ 192, 168, 1, 1 }};
IPAdress subnet                 = {{ 255, 255, 0, 0 }};

// IP configuration of known IPs
IPAdress ip_known_client       = {{ 192, 168, 1, 2  }};
IPAdress ip_scan_start         = {{ 192, 168, 1, 0 }};
IPAdress ip_scan_end           = {{ 192, 168, 1, 255 }};
IPAdress ip_to_scan            = ip_scan_start;

// IPAdresses ip_found_clients;
IPAdresses ip_found_clients;

// Counter for the loop
int i                         = 0;

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
  Ethernet.begin(mac_shield, ip_shield.ipadress, gateway.ipadress, subnet.ipadress);
  
  
  // Setup when no IP is known
  if (Ethernet.begin(mac_shield) == 0) {
    Serial.println("DHCP failed, no automatic IP address assigned!");
    Serial.println("Time for waiting for IP address: "+millis());
    Serial.println("Trying to set manual IP address: 10.0.1.10...");
    static uint8_t ip[4]    = { 10, 0, 1, 13 };
    Ethernet.begin(mac_shield, ip);
  }
  

  Serial.println(" Address assigned?");
  Serial.print(" ");
  Serial.println(Ethernet.localIP());
  Serial.println(" Setup complete\n");
  // Setup End
  
  
  
  // Testing known address
  ping(1, ip_known_client.ipadress, buffer);
  Serial.println(buffer);
  readable_ip(ip_known_client.ipadress, "Testing known address %s"); 


  // Pinging first address of address range
  ping(1, ip_scan_start.ipadress, buffer);
  Serial.println((String) buffer);
  readable_ip(ip_known_client.ipadress, "Testing first address %s of address range"); 
  
  
  Serial.println("\n");
  Serial.println("Starting loop trough IP range");
  
  // Adding Known Ip to known IPs
  insertArray(&ip_found_clients, ip_known_client);  
  
}



void loop() {
  // readable_ip(ip_to_scan, "IP before upcounting: %s\n");
  i++;
  if (i < 255) {
    ip_to_scan.ipadress[3] = (byte)(ip_to_scan.ipadress[3] + 1);
    /*
    if (ip_to_scan[3] == 0) {
      ip_to_scan[2] = (byte)(ip_to_scan[2] + 1);
      if (ip_to_scan[2] == 0) {
        ip_to_scan[1] = (byte)(ip_to_scan[1] + 1);
        if (ip_to_scan[1] == 0) {
          ip_to_scan[0] = (byte)(ip_to_scan[0] + 1);
        }
      }
    }*/
    //readable_ip(ip_to_scan, "IP after upcounting: %s\n");


    Serial.println("\n");

    // Todo check for more known IPs (gateway, ...)
    if (byte_to_readable_ip_string(ip_to_scan.ipadress) == byte_to_readable_ip_string(ip_shield.ipadress)) {
      readable_ip(ip_to_scan.ipadress, "Ignoring %s, it's my own IP.");
      return; // "continue" for the main loop()
    }
    
    ping(1, ip_to_scan.ipadress, buffer);
    
    String ping_result = buffer;
    //Serial.println(ping_result);
    
    if (ping_result.indexOf("Timed Out") == -1) {
      // We found a device!
      insertArray(&ip_found_clients, ip_to_scan);  
      readable_ip(ip_found_clients.array[ip_found_clients.used-1].ipadress, "Device found on: %s");
      remove_element(&ip_found_clients, 0);
      
      
      
    } else {
      // It's not responding, next one
      readable_ip(ip_to_scan.ipadress, "No (pingable) device on IP: %s");
    }
    
  } else {
    Serial.println("\nRestart from .0");
    i = 0;
  }
}




