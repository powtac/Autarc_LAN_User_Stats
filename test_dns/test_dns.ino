#include <SPI.h>
#include <Ethernet.h>
#include <Dns.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetClient client;

char server[] = "kolchose.org";
int serverport = 80;

void setup() {
  // start serial port:
  Serial.begin(115200);
  // give the ethernet module time to boot up:
  delay(1000);
  // start the Ethernet connection using a fixed IP address and DNS server:
  //Ethernet.begin(mac, ip, ipdns, ipgw, mask);
  Ethernet.begin(mac);
  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
 
  DNSClient dns;
  // OPENDNS
  // IPAddress dns_ip(208, 67, 222, 222);
  IPAddress dns_ip(10, 0, 1, 1);
  
  IPAddress out_ip;
  dns.begin(dns_ip);
  
  dns.getHostByName(server, out_ip);
  
  Serial.println(out_ip);
  
  delay(5000);

}
