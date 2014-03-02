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
static char AVRID[6]           = "Tim1";
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
byte currIP[4];
// Array for found devices
byte found[35] [6]; // TODO: Auf 256 setzen -> found[ip] [MAC-Blöcke]

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
      //TODO: Ask for Ip, Gateway, Netmask to configurate
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
  

  for (byte i = 0; i < 4; i++) {
    readSubnet[i] = Ethernet.subnetMask()[i], DEC;
    readIP[i]     = Ethernet.localIP()[i], DEC;
    start_ip[i]   = readIP[i] & readSubnet[i];
  }
  
  // TODO into one line
  Serial.print("\nStarting loop trough IP range ");
  print_ip(start_ip);
  Serial.print(" - ");
  // Serial.print(byte_to_readable_ip_string(ip_scan_end));
  Serial.print("...\n");
}

void loop() {
  Serial.print("Speicher (Loop-Start): ");
  Serial.println(get_mem_unused());
  for(int b = 0; b < 4; b++) { 
    currIP[b]  = start_ip[b]; 
  }
  while (1) {
    if (currIP[3] < 35) { // TODO: An mögliche über SubnetMask anpassen  //255
      ICMPEchoReply echoReply = ping(currIP, pingrequest);    
      if (echoReply.status == SUCCESS) {
        // We found a device!
        Serial.print("Speicher (Geraet gefunden): ");
        Serial.println(get_mem_unused());
        for(int mac = 0; mac < 6; mac++) {
          found[currIP[3]][mac] = echoReply.MACAddressSocket[mac];
        }
        
        Serial.print("Device found on: ");
        print_ip(currIP);
        Serial.print(" MAC: ");
        print_mac(echoReply.MACAddressSocket);
        
      } else {
        // It's not responding, next one
        for(int mac = 0; mac < 6; mac++) {
          found[currIP[3]][mac] = 0;
        }
        Serial.print("No (pingable) device on IP ");
        print_ip(currIP);
      }
      currIP[3]++;  
    } else {
      // next IP-Block?!
      break; // Exit Loop 
    }
  } 
  Serial.print("Speicher (Ende Suchvorgang): ");
  Serial.println(get_mem_unused());
  Serial.println("Fertig");
  
  for(int z = 0; z < 35; z++) {  // TODO: Set to 255
    if (found[z] [5] > 0) {    // TODO: Richtig filtern!
      Serial.print("Found: IP: ");
      Serial.print(start_ip[0]);
      Serial.print(".");
      Serial.print(start_ip[1]);
      Serial.print(".");
      Serial.print(start_ip[2]);
      Serial.print(".");
      Serial.print(z);
      Serial.print(" - MAC: ");
      print_mac(found[z]);
    }
  }
  Serial.print("Speicher (Ende Loop): ");
  Serial.println(get_mem_unused());
  
  send_info_to_server(start_ip, found, AVRID);
  
  Serial.print("Speicher (Ende ServerSend): ");
  Serial.println(get_mem_unused());
   
  Serial.println("Restart loop");
}
