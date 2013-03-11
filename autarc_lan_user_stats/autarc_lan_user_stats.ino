
#include <SPI.h>
#include <Ethernet.h> 
#include "ICMPPing.h"

#include "IPHelper.h"


// See http://playground.arduino.cc/Main/Printf
#include <stdarg.h>
void p(char *fmt, ... ){
        char tmp[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(tmp, 128, fmt, args);
        va_end (args);
        Serial.print(tmp);
}

void printReadableIPAddress();

SOCKET pingSocket = 0;
char buffer [256];
String str;  

boolean firstrun = false;

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

  //ICMPPing ping(pingSocket);

void readable_ip(int ip, char* message) {
  p(message, int_to_readable_ip_string(ip));
}

void readable_ip(byte* ip, char* message) {
  String readable_ip_string = byte_to_readable_ip_string(ip);
  char readable_ip[15]; 
  
  readable_ip_string.toCharArray(readable_ip, sizeof(readable_ip));
  
  p(message, readable_ip);
}

const char int_to_readable_ip_string(int ip) {
  unsigned char bytes[4];
  bytes[0] = ip & 0xFF;
  bytes[1] = (ip >> 8) & 0xFF;
  bytes[2] = (ip >> 16) & 0xFF;
  bytes[3] = (ip >> 24) & 0xFF;

  const char readable_ip = printf("%d.%d.%d.%d", bytes[3], bytes[2], bytes[1], bytes[0]);
  
  Serial.println("HIER");
  Serial.println(readable_ip);
  delay(5000);
  
  return readable_ip;
}

String byte_to_readable_ip_string(byte* ip) {
  String ip_readable;
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    ip_readable.concat((ip[thisByte]));
    if (thisByte != 3) {
      ip_readable.concat(".");
    }
  }
  
  return ip_readable;
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
  Serial.println("\n");

  ICMPPing ping(pingSocket);
  ping(1, known_client_ip, buffer);
  Serial.print("Reference ping: ");
  Serial.println(buffer);
  delay(500);
  
  readable_ip(known_client_ip, "Testing known address %s"); 

  ping(1, addr, buffer);
  Serial.println((String) buffer);  
  
  Serial.println("\n");
  Serial.println("\n");
  Serial.println("Starting loop trough IP range");
  Serial.println("\n");
  
  //
}

ICMPPing ping(pingSocket);
void loop() {

  i++;  
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


    Serial.println("\n");

    // Todo check for more known IPs (gateway, ...)
    if (byte_to_readable_ip_string(addr) == byte_to_readable_ip_string(ip_shield)) {
      readable_ip(addr, "Ignoring %s, it's my own IP.");
      return; // "continue" for the main loop()
    }
    
    ping(1, addr, buffer);
    
    String ping_result = buffer;
    Serial.println(ping_result);
    
    if (ping_result.indexOf("Timed Out") == -1) {
      readable_ip(addr, "Device found on: %s");
    } else {
      readable_ip(addr, "No (pingable) device on IP: %s");
    }
    
  } else {
    Serial.println("Restart from .0");
    i = 0;
  }
}





