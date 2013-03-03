
#include <SPI.h>
#include <Ethernet.h> 
#include "ICMPPing.h"

#include "IPHelper.h"


void printReadableIPAddress();

SOCKET pingSocket = 0;
char buffer [256];
String str;  

// please modify the following two lines. mac and ip have to be unique
// in your local area network. You can not have the same numbers in
// two devices:
// WireShark Filter: eth.addr[0:3]==90:A2:DA
static uint8_t mac[6]   = { 
  0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F }; 
  
  
  
static uint8_t ip[4]    = { 10, 0, 1, 13 };
byte ip_shield[]        = { 10, 0, 1, 13 };    
// the router's gateway address:
byte gateway[] = { 10, 0, 0, 1 };
// the subnet:
byte subnet[] = { 255, 255, 0, 0 };


byte known_client_ip[4] = { 
  10, 0, 1, 3 };
byte scan_start_ip[4]   = { 
  10, 0, 1, 0 };
byte scan_end_ip[4]     = { 
  10, 0, 1, 255 };

int i                   = 0;  
byte * addr = {
  scan_start_ip};

#define BUFFER_SIZE 250
unsigned char buf[BUFFER_SIZE+1];

uint16_t plen;

// EtherShield es=EtherShield();



void readable_ip(int ip, const char* message) {
  // const char readable_ip = ;
  Serial.println(printf(message, int_to_readable_ip_string(ip)));
}

void readable_ip(byte* ip, const char* message) {
  Serial.println(printf(message, byte_to_readable_ip_string(ip)));
}

const char int_to_readable_ip_string(int ip) {
  unsigned char bytes[4];
  bytes[0] = ip & 0xFF;
  bytes[1] = (ip >> 8) & 0xFF;
  bytes[2] = (ip >> 16) & 0xFF;
  bytes[3] = (ip >> 24) & 0xFF;
  
  const char readable_ip = printf("%d.%d.%d.%d", bytes[3], bytes[2], bytes[1], bytes[0]);
  return readable_ip;
}

const char byte_to_readable_ip_string(byte* ip) {
  
  const char readable_ip = printf("%d.%d.%d.%d", ip[3], ip[2], ip[1], ip[0]);
  return readable_ip;
  /*
  String ip_readable;
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    ip_readable.concat((ip[thisByte]));
    if (thisByte != 3) {
      ip_readable.concat(".");
    }
  }
  return ip_readable;*/
}

void setup() {

  Serial.begin(9600);

  Serial.println("Try to get IP address...");
  
  Ethernet.begin(mac, ip_shield, gateway, subnet);
  
  /*
  if (Ethernet.begin(mac, ip_shield, gateway, subnet) == 0) {
    Serial.println("DHCP failed, no automatic IP address assigned!");
    Serial.println("Time for waiting for IP address: "+millis());
    Serial.println("Trying to set manual IP address: 10.0.1.10...");
    Ethernet.begin(mac, ip);
  }
  */

  Serial.println("Address assigned?");
  Serial.println(Ethernet.localIP());
  Serial.println("Setup complete");



  readable_ip(addr, "The start address: %s");
  /*
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(addr[thisByte], DEC);
    Serial.print(".");
  }*/
  Serial.println();

  ICMPPing ping(pingSocket);
  ping(4, known_client_ip, buffer);
  Serial.print("Reference ping: ");
  Serial.println(buffer);
  delay(500);
  
  /*
  Serial.print("Testing known address");*/
  readable_ip(known_client_ip, "Testing known address %s"); 

  // ICMPPing ping(pingSocket);
  ping(4, addr, buffer);
  str = (String) buffer;
  Serial.println(buffer);  

  Serial.print("Starting loop trough IP range");
}


void loop() {

  i++;
  ICMPPing ping(pingSocket);
  // Serial.println(i);
  if (i < 255) {
    
    addr[3] = (byte)(addr[3] + 1);
    if (addr[3] == 0) {
      addr[2] = (byte)(addr[2] + 1);
      if (addr[2] == 0) {
        addr[1] = (byte)(addr[1] + 1);
        if (addr[1] == 0) {
          addr[0] = (byte)(addr[0] + 1);
        }
      }
    }


    /*
    Serial.print("End IP: ");
    for (byte thisByte = 0; thisByte < 4; thisByte++) {
      // print the value of each byte of the IP address:
      Serial.print(scan_end_ip[thisByte], DEC);
      Serial.print(".");
    }
    */
    
    
    ping(4, addr, buffer);
    

    
    str = (String) buffer;
    
    Serial.println(buffer);
    
    if (str.indexOf("Timed Out:") == 0) {
      Serial.println(buffer);
    } else {
      
      /*
      Serial.print("No (pingable) device on IP: ");*/
      readable_ip(addr, "No (pingable) device on IP: %s");
      /*
      for (byte thisByte = 0; thisByte < 4; thisByte++) {
        // print the value of each byte of the IP address:
        Serial.print(addr[thisByte], DEC);
        Serial.print(".");
      }*/
      Serial.println();
    }
    
    // delay(500);
  } else {
    
  }
}





