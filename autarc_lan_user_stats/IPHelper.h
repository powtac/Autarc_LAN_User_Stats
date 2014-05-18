// Some helpers for TCP/IP stuff

#include <stdarg.h>
#include "memcheck.h"
// Serial.println(get_mem_unused());

void print_ip(byte* ip) {
  Serial.print(ip[0]);
  Serial.print(".");
  Serial.print(ip[1]);
  Serial.print(".");
  Serial.print(ip[2]);
  Serial.print(".");
  Serial.println(ip[3]);
}


void print_mac(byte* mac) {
  Serial.print(mac[0], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.println(mac[5], HEX);
}


void write_EEPROM(int startstorage, byte *value, int valuesize)
{
   for (int i = 0; i < valuesize; i++) {
     EEPROM.write(i + startstorage, value[i]);
   }
}

void write_EEPROM(int startstorage, char *value, int valuesize)
{
   for (int i = 0; i < valuesize; i++) {
     EEPROM.write(i + startstorage, value[i]);
   }
}

void write_EEPROM(int startstorage, byte value)
{
    EEPROM.write(startstorage, value);
}


void read_EEPROM(int startstorage, byte *value, int valuesize)
{
   for (int i = 0; i < valuesize; i++) {
     value[i] = EEPROM.read(i + startstorage);
   }
}

void read_EEPROM(int startstorage, char *value, int valuesize)
{
   for (int i = 0; i < valuesize; i++) {
     value[i] = EEPROM.read(i + startstorage);
   }
}

byte read_EEPROM(int startstorage)
{
   return EEPROM.read(startstorage);
}


void GetString(char *buf, int bufsize)
{
  //TODO: Correction not possible yet
  int i;
  char ch;
  for (i=0; i<bufsize - 1; ++i)
  {
    while (Serial.available() == 0); // wait for character to arrive
    ch = Serial.read();
    Serial.print(ch);
    if (ch == '\r') {
     buf[i] = 0; // 0 string terminator just in case
     Serial.println("\n");
     break;
    }
    else {
      buf[i] = ch;
    }
  }
}


byte GetNumber(void)
{
  char input[2];
  GetString(input, sizeof(input));
  return atoi(input);
}


void GetIP(byte *IP)
{
  char input[16];
  GetString(input, sizeof(input));
 
  char *i;
  IP[0] = atoi(strtok_r(input,".",&i));
  IP[1] = atoi(strtok_r(NULL,".",&i));
  IP[2] = atoi(strtok_r(NULL,".",&i));
  IP[3] = atoi(strtok_r(NULL,".",&i));
}


void GetMAC(byte *MAC)
{
  char input[18];
  GetString(input, sizeof(input));
 
  char *i;
  MAC[0] = strtol(strtok_r(input,":",&i), NULL, 16);
  MAC[1] = strtol(strtok_r(NULL,":",&i), NULL, 16);
  MAC[2] = strtol(strtok_r(NULL,":",&i), NULL, 16);
  MAC[3] = strtol(strtok_r(NULL,":",&i), NULL, 16);
  MAC[4] = strtol(strtok_r(NULL,":",&i), NULL, 16);
  MAC[5] = strtol(strtok_r(NULL,":",&i), NULL, 16);
}


void send_info_to_server(byte* IP, byte* MAC, char* AVRID) {
  // int result = Ethernet.maintain(); // renew DHCP
  // Serial.print(" DHCP renew:");
  // Serial.println(result); 
  // delay(1000);
  Serial.print("Speicher (send_info): ");
  Serial.println(get_mem_unused());
  EthernetClient client;
  // IPAddress server(85, 10, 211, 16); // kolchose.org
  //TODO: Is the IP required?
  IPAddress server(85, 214, 144, 114); //lan-user.danit.de
  
  // int ret = client.connect("kolchose.org", 80);
  int ret = client.connect("lan-user.danit.de", 80);
  if (ret == 1) {
    Serial.println("Connected to HTTP Server");

    // Make a HTTP request:
    // client.print("GET /autarc_lan_user_stats/"); // kolchose.org 
    client.print("GET /"); // lan-user.danit.de
    client.print("?AVR_ID=");
    client.print(AVRID);
    Serial.println(AVRID);
    // HTTP String:  AVR_ID=AVR_ID&IP[]=Ip&MAC[]=Mac&IP[]=IP&MAC[]=Mac
        client.print("&IP[]=");
        client.print(IP[0]);
        client.print(".");
        client.print(IP[1]);
        client.print(".");
        client.print(IP[2]);
        client.print(".");
        client.print(IP[3]);
        client.print("&MAC[]=");
        client.print(MAC[0], HEX);
        client.print(":");
        client.print(MAC[1], HEX);
        client.print(":");
        client.print(MAC[2], HEX);
        client.print(":");
        client.print(MAC[3], HEX);
        client.print(":");
        client.print(MAC[4], HEX);
        client.print(":");
        client.print(MAC[5], HEX);

    client.println(" HTTP/1.0");
    // client.println("Host: kolchose.org"); // Important! TODO check if this is required and dynamically asignable
    client.println("Host: lan-user.danit.de");
    client.println("User-Agent: Autarc_LAN_User_Stats"); // Important!
    client.println(); // Important!
    
    Serial.println(client.status());
    client.stop();

  }
  else {
    Serial.println("NOT connected to HTTP Server");
    Serial.println(ret);
    Serial.println("\n");
    Serial.println(client.status());
    client.stop();
  }
}
