// Some helpers for TCP/IP stuff

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


void readable_ip(int ip, char* message) {
  p(message, int_to_readable_ip_string(ip));
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


void readable_ip(byte* ip, char* message) {
  String readable_ip_string = byte_to_readable_ip_string(ip);
  char readable_ip[15]; 
  
  readable_ip_string.toCharArray(readable_ip, sizeof(readable_ip));
  
  p(message, readable_ip);
}

// Required?
/*
void printReadableIPAddress() {
  Serial.begin(9600);
  Serial.print("IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print("."); 
  }
}
*/

void readable_mac(byte* mac, char* message) {
  char mac_string[18];
  snprintf(mac_string, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); 
  p(message, mac_string);
}

void readable_mac(String mac, char* message) {
  p(message, mac);
}

void send_info_to_server(String info) {
  
}



