
#include <SPI.h>
#include <Ethernet.h> 
#include "ICMPPing.h"


SOCKET pingSocket = 0;
char buffer [256];
String str;  

// please modify the following two lines. mac and ip have to be unique
// in your local area network. You can not have the same numbers in
// two devices:
static uint8_t mac[6]   = { 
  0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F }; 
static uint8_t ip[4]    = { 
  10, 0, 1, 13 };

byte known_client_ip[4] = { 
  10, 0, 1, 8 };
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



void int_to_readable_ip_string(int ip) {
  unsigned char bytes[4];
  bytes[0] = ip & 0xFF;
  bytes[1] = (ip >> 8) & 0xFF;
  bytes[2] = (ip >> 16) & 0xFF;
  bytes[3] = (ip >> 24) & 0xFF;	
  Serial.print(printf("%d.%d.%d.%d\n", bytes[3], bytes[2], bytes[1], bytes[0]));        
}

void setup() {

  Serial.begin(9600);

  Serial.println("Try to get IP address...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP failed, no automatic IP address assigned!");
    Serial.println("Time for waiting for IP address: "+millis());
    Serial.println("Trying to set manual IP address: 10.0.1.10...");
    Ethernet.begin(mac, ip);
  }

  Serial.println("Address assigned?");
  Serial.println(Ethernet.localIP());
  Serial.println("Setup complete");



  Serial.print("The start address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(addr[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();

  ICMPPing ping(pingSocket);
  ping(4, known_client_ip, buffer);
  Serial.print("Reference ping: ");
  Serial.println(buffer);
  delay(500);

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
    
    if ( str.indexOf("+Request Timed Out:") == 0) {
      Serial.println(buffer);
    } else {
      Serial.print("No (pingable) device on IP: ");
      for (byte thisByte = 0; thisByte < 4; thisByte++) {
        // print the value of each byte of the IP address:
        Serial.print(addr[thisByte], DEC);
        Serial.print(".");
      }
      Serial.println();
    }
    
    // delay(500);
  } else {
    
  }
}





