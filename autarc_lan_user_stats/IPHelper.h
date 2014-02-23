// Some helpers for TCP/IP stuff

#include <stdarg.h>
#include "memcheck.h"
  //Serial.println(get_mem_unused());

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
  Serial.print(mac[0]);
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


typedef byte (*PtrArray)[6];

void send_info_to_server(byte* startip, PtrArray maclist, char* AVRID) {
  //int result = Ethernet.maintain(); // renew DHCP
  //Serial.print(" DHCP renew:");
  //Serial.println(result); 
  //delay(1000);
  Serial.print("Speicher (send_info): ");
  Serial.println(get_mem_unused());
  EthernetClient client;
  //IPAddress server(85, 10, 211, 16); // kolchose.org 
  IPAddress server(85, 214, 144, 114); //lan-user.danit.de
  
  //int ret = client.connect("kolchose.org", 80);
  int ret = client.connect("lan-user.danit.de", 80);
  if (ret == 1) {
    Serial.println("Connected to HTTP Server");

    // Make a HTTP request:
    //client.print("GET /autarc_lan_user_stats/"); // kolchose.org 
    client.print("GET /"); // lan-user.danit.de
    client.print("?AVR_ID=");
    client.print(AVRID);
    Serial.println(AVRID);
    //HTTP String:  AVR_ID=AVR_ID&IP[]=Ip1&MAC[]=Mac2&IP[]=IP2&MAC[]=Mac2
    for(int z = 0; z < 35; z++) {  //Todo: Set to 255
      //if (maclist[z] [5] > 0) {    //Todo: Richtig filtern!
        client.print("&IP[]=");
        client.print(startip[0]);
        client.print(".");
        client.print(startip[1]);
        client.print(".");
        client.print(startip[2]);
        client.print(".");
        client.print(z);
        client.print("&MAC[]=");
        client.print(maclist[z][0], HEX);
        client.print(".");
        client.print(maclist[z][1], HEX);
        client.print(".");
        client.print(maclist[z][2], HEX);
        client.print(".");
        client.print(maclist[z][3], HEX);
        client.print(".");
        client.print(maclist[z][4], HEX);
        client.print(".");
        client.print(maclist[z][5], HEX);
      //}
    }

    client.println(" HTTP/1.0");
    //client.println("Host: kolchose.org");           // Important! TODO check if this is required and dynamically asignable
    client.println("Host: lan-user.danit.de");
    client.println("User-Agent: Autarc_LAN_User_Stats"); // Important!
    client.println();        // Important!
    
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
