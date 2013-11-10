#include <SPI.h>
#include <Ethernet.h> 


EthernetClient client;
// IPAddress server(64, 233, 187, 99); // Google
IPAddress server(85, 10, 211, 16); // kolchose.org 

static uint8_t mac_shield[6]   = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F };

void setup() {
  delay(1000);
  if (Ethernet.begin(mac_shield) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    for(;;)
      ;
  }

  delay(1000);

  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  int result = Ethernet.maintain(); // renew DHCP
  Serial.print(" DHCP renew:");
  Serial.println(result);

  delay(1000);
}

void loop() {
  int ret = client.connect("kolchose.org", 80);
  if (ret == 1) {
    Serial.println("Connected to HTTP Server");

    // Make a HTTP request:
    client.println("GET /autarc_lan_user_stats/?test HTTP/1.0");
    client.println("Host: kolchose.org");           // Important!
    client.println("User-Agent: arduino-ethernet"); // Important!
    client.println();                               // Important!

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
  delay(800);
  /*
  int result = Ethernet.maintain(); // renew DHCP
   Serial.print(" DHCP renew:");
   Serial.println(result);
   */
}

