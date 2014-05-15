#include <SPI.h>
#include <Ethernet.h> 
// See https://github.com/BlakeFoster/Arduino-Ping/
#include "ICMPPing.h"
#include "IPHelper.h"
#include "memcheck.h"
// init_mem();  //hier???
// Serial.println(get_mem_unused());

// Shield and network configuration
// WireShark Filter: eth.addr[0:3]==90:A2:DA
boolean useDhcp                = true; // Using DHCP? If no please set ip_shield, gateway and subnet below
char pingrequest               = 2;

//// Tim
static char AVRID[6]           = "Tim2";
static uint8_t mac_shield[6]   = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x9F };
byte ip_shield[4]              = { 192, 168, 178, 98 };
byte gateway[4]                = { 192, 168, 178, 1 };
byte subnet[4]                 = { 255, 255, 255, 0 };

// Simon
//static char AVRID[6]           = "Simon";
//static uint8_t mac_shield[6]   = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F };
//byte ip_shield[4]              = { 10, 0, 1, 13 };
//byte gateway[4]                = { 10, 0, 1, 1 };
//byte subnet[4]                 = { 255, 255, 0, 0 };

//// Jonas
//static char AVRID[6]           = "Jonas";
//static uint8_t mac_shield[6]   = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F };
//byte ip_shield[4]              = { 192, 168, 1, 30 };
//byte gateway[4]                = { 192, 168, 1, 1 };
//byte subnet[4]                 = { 255, 255, 0, 0 };


byte start_ip[4];
byte end_ip[4];
byte currIP[4];
byte currMAC[6];

byte readSubnet[4];
byte readIP[4];

int configurate;

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
  Serial.print("Speicher: ");
  Serial.println(get_mem_unused());
  Serial.print("Press any key to configurate");
  for (char i = 0; i < 3 and Serial.available() <= 0; i++) {
    delay(1000);
    Serial.print(".");
  }
  configurate = Serial.read();
  if (configurate >= 0) {
    Serial.println("Starting configuration");
      Serial.println("MAC Board: ");
      GetMAC(mac_shield);
      //TODO: Save in EEPROM
      print_mac(mac_shield);
      Serial.println("Stored");
      Serial.println("\n");
      
      Serial.println("IP Board: ");
      GetIP(ip_shield);
      //TODO: Save in EEPROM
      print_ip(ip_shield);
      Serial.println("Stored");
      Serial.println("\n");
      
      Serial.println("IP Gateway: ");
      GetIP(gateway);
      //TODO: Save in EEPROM
      print_ip(gateway);
      Serial.println("Stored");
      Serial.println("\n");
      
      Serial.println("Subnetmask: ");
      GetIP(subnet);
      //TODO: Save in EEPROM
      print_ip(subnet);
      Serial.println("Stored");
      Serial.println("\n");
      
      Serial.println("\n");
      Serial.println("Setup finished");
      Serial.println("\n");
      
    } else {
    Serial.println("no configuration");
  }
  
  // Setup Start
  Serial.println("Try to get IP address from network...");
  Serial.print(" MAC address of shield: ");
  print_mac(mac_shield);
  // Setup when no IP is known
  if (useDhcp == false) {
    Ethernet.begin(mac_shield, ip_shield, gateway, subnet);
  } else {
    if (Ethernet.begin(mac_shield) == 0) {
      Serial.println("DHCP failed, no automatic IP address assigned!");
      Serial.print("Time for waiting for IP address: ");
      Serial.print(millis());
      Serial.println(" ms");
      Serial.println("Trying to set manual IP address.");
      Ethernet.begin(mac_shield, ip_shield, gateway, subnet);
    }
  }

  Serial.println(" Address assigned?");
  Serial.print(" ");
  Serial.println(Ethernet.localIP());
  Serial.print(" ");
  Serial.println(Ethernet.subnetMask());
  Serial.println(" Setup complete\n");
  Serial.print("Speicher: ");
  Serial.println(get_mem_unused());
  // Setup End
  
//TODO: Check for Subnetting!
  for (byte i = 0; i < 4; i++) {
    readSubnet[i] = Ethernet.subnetMask()[i], DEC;
    readIP[i]     = Ethernet.localIP()[i], DEC;
    start_ip[i]   = readIP[i] & readSubnet[i];
    if (start_ip[i] == 0) {
     start_ip[i] = 1;   //TODO: Set to 2, because of Gateway
    }
    end_ip[i]     = readIP[i] | ~readSubnet[i];
    if (end_ip[i] == 255) {
     end_ip[i] = 254; 
    }
   }
  
  Serial.print("\nStarting loop trough IP range ");
  print_ip(start_ip);
  Serial.print(" - ");
  print_ip(end_ip);
}

void loop() {
  Serial.print("Speicher (Loop-Start): ");
  Serial.println(get_mem_unused());
  for(int b = 0; b < 4; b++) { 
    currIP[b]  = start_ip[b]; 
  }
  while (1) {
    if (currIP[3] <= end_ip[3]) { // TODO: An mögliche über SubnetMask anpassen  //255
      ICMPEchoReply echoReply = ping(currIP, pingrequest);    
      if (echoReply.status == SUCCESS) {
        // We found a device!
        Serial.print("Speicher (Geraet gefunden): ");
        Serial.println(get_mem_unused());
        for(int mac = 0; mac < 6; mac++) {
          currMAC[mac] = echoReply.MACAddressSocket[mac];
        }
        
        Serial.print("Device found on: ");
        print_ip(currIP);
        Serial.print(" MAC: ");
        print_mac(currMAC);
        
      } else {
        // It's not responding, next one
        for(int mac = 0; mac < 6; mac++) {
          currMAC[mac] = 0;
        }
        Serial.print("No (pingable) device on IP ");
        print_ip(currIP);
      }
      send_info_to_server(currIP, currMAC, AVRID);
      
      currIP[3]++;  
    } else {
      // next IP-Block?!
      break; // Exit Loop 
    }
  }
  Serial.print("Speicher (Ende ServerSend): ");
  Serial.println(get_mem_unused());
  Serial.println("Restart loop");
}
