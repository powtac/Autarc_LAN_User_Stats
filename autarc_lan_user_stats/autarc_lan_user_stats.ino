
#include <SPI.h>
#include <Ethernet.h> 
#include "ICMPPing.h"
#include "IPHelper.h"



// Shield and network configuration
// WireShark Filter: eth.addr[0:3]==90:A2:DA
static uint8_t mac_shield[6]  = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F };
byte ip_shield[]              = { 10, 0, 1, 13 };    
byte gateway[]                = { 10, 0, 0, 1 };
byte subnet[]                 = { 255, 255, 0, 0 };


// IP configuration of known IPs
byte ip_known_client[4]       = { 10, 0, 1, 3 };
byte ip_scan_start[4]         = { 10, 0, 1, 0 };
byte ip_scan_end[4]           = { 10, 0, 1, 255 };
byte * ip_to_scan             = { ip_scan_start };


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

  Ethernet.begin(mac_shield, ip_shield, gateway, subnet);
  
  /*
  // Setup when no IP is known
  if (Ethernet.begin(mac_shield, ip_shield, gateway, subnet) == 0) {
    Serial.println("DHCP failed, no automatic IP address assigned!");
    Serial.println("Time for waiting for IP address: "+millis());
    Serial.println("Trying to set manual IP address: 10.0.1.10...");
    static uint8_t ip[4]    = { 10, 0, 1, 13 };
    Ethernet.begin(mac_shield, ip);
  }
  */

  Serial.println("Address assigned?");
  Serial.println(Ethernet.localIP());
  Serial.println("Setup complete\n");
  // Setup End
  
  
  
  // Testing known address
  ping(1, ip_known_client, buffer);
  Serial.println(buffer);
  readable_ip(ip_known_client, "Testing known address %s"); 


  // Pinging first address of address range
  ping(1, ip_scan_start, buffer);
  Serial.println((String) buffer);
  readable_ip(ip_known_client, "Testing first address %s of address range"); 
  
  
  Serial.println("\n");
  Serial.println("\n");
  Serial.println("Starting loop trough IP range");
  Serial.println("\n");
}



void loop() {
  
  i++;  
  if (i < 255) {
    ip_to_scan[3] = (byte)(ip_to_scan[3] + 1);
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


    Serial.println("\n");

    // Todo check for more known IPs (gateway, ...)
    if (byte_to_readable_ip_string(ip_to_scan) == byte_to_readable_ip_string(ip_shield)) {
      readable_ip(ip_to_scan, "Ignoring %s, it's my own IP.");
      return; // "continue" for the main loop()
    }
    
    ping(1, ip_to_scan, buffer);
    
    String ping_result = buffer;
    Serial.println(ping_result);
    
    if (ping_result.indexOf("Timed Out") == -1) {
      readable_ip(ip_to_scan, "Device found on: %s");
    } else {
      readable_ip(ip_to_scan, "No (pingable) device on IP: %s");
    }
    
  } else {
    Serial.println("\nRestart from .0");
    i = 0;
  }
}





