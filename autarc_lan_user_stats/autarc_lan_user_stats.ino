#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
// See https://github.com/BlakeFoster/Arduino-Ping/
#include "ICMPPing.h"
#include "IPHelper.h"
#include "memcheck.h"
#include "default_config.h"
#include "TimerOne.h"
// init_mem();  //hier???
// Serial.println(get_mem_unused());

// WireShark Filter: eth.addr[0:3]==90:A2:DA

//// Tim

//EEPROM
// Storenumber |    Variable   | Size
//-----------------------------------
//  0          | configured    | 1
//  1 -  6     | mac_shield    | 6
//  7 - 10     | ip_shield     | 4
// 11 - 14     | gateway       | 4
// 15 - 18     | subnet        | 4
// 19          | useDhcp       | 1
// 20          | pingrequest   | 1
// 21          | useSubnetting | 1
// 22 - 25     | start_ip      | 4
// 26 - 29     | end_ip        | 4
// 30 - 35     | AVRID         | 6


//Todo: It's only temporarily. Add to configuration?!
byte dnsSrv[4] = { 192, 168, 178, 1 };

byte readSubnet[4];
byte readIP[4];

byte currIP[4];
byte currMAC[6];

int configuration;

const char string_format_ip[] = ", format \"000.111.222.333\": ";
const char string_stored[] = " stored";

// Ping library configuration
SOCKET pingSocket              = 0;

// HTTP server in the internet
// EthernetClient client;
// IPAddress server(64, 233, 187, 99);
// IPAddress server(85,10,211,16); // kolchose.org

// Global namespace to use it in setup() and loop()
ICMPPing ping(pingSocket, (uint16_t)random(0, 255));

EthernetServer server(80);

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.print(F("Memory: "));
  Serial.println(get_mem_unused());
  
//________________________Configuration of the board______________________________

  Serial.print("Press any key start configuration");
  for (char i = 0; i < 5 and Serial.available() <= 0; i++) {
    delay(1000);
    Serial.print(".");
  }
  
  configuration = Serial.read();
  if (configuration >= 0) {
    Serial.println(F("Starting configuration"));
    
    Serial.println(F("Load default configuration (0 = no): "));
    if (GetNumber() == 0) {    
      Serial.println(F("MAC Board, format \"00:00:00:00:00:00\": "));
      GetMAC(mac_shield);
      write_EEPROM(1, mac_shield , sizeof(mac_shield));
      Serial.println();
      print_mac(mac_shield);
      Serial.println(string_stored);
      Serial.println("\n");
      
      Serial.print(F("IP Board"));
      Serial.println(string_format_ip);
      GetIP(ip_shield);
      write_EEPROM(7, ip_shield , sizeof(ip_shield));
      Serial.println();
      print_ip(ip_shield);
      Serial.println(string_stored);
      Serial.println("\n");
      
      Serial.print(F("IP Gateway"));
      Serial.println(string_format_ip);
      GetIP(gateway);
      write_EEPROM(11, gateway , sizeof(gateway));
      Serial.println();
      print_ip(gateway);
      Serial.println(string_stored);
      Serial.println("\n");
      
      //TODO: Check if it works fine!
      Serial.print(F("Subnetmask"));
      Serial.println(string_format_ip);
      GetIP(subnet);
      write_EEPROM(15, subnet , sizeof(subnet));
      Serial.println();
      print_ip(subnet);
      Serial.println(string_stored);
      Serial.println("\n");
      
      Serial.println(F("Use DHCP (0 = no): "));
      useDhcp = GetNumber();
      write_EEPROM(19, useDhcp);
      if (useDhcp == 0) {
        Serial.println(F("Don't use DHCP"));
      } else {
        Serial.println(F("Use DHCP"));
      }
      Serial.println(string_stored);
      Serial.println("\n");
      
      Serial.println(F("Number of ping-requests: "));
      pingrequest = GetNumber();
      write_EEPROM(20, pingrequest);
      Serial.print(F("Number of ping-requests: "));
      Serial.print(pingrequest);
      Serial.println(string_stored);
      Serial.println("\n");
      
      Serial.println(F("Use Subnetting (0 = no): "));
      useSubnetting = GetNumber();
      write_EEPROM(21, useSubnetting);
      if (useSubnetting == 0) {
        Serial.println(F("Don't use Subnetting"));
        Serial.println(string_stored);
        Serial.println("\n");
         
        Serial.print(F("Start IP for scan"));
        Serial.println(string_format_ip);
        GetIP(start_ip);
        write_EEPROM(22, start_ip , sizeof(start_ip));
        Serial.println();
        print_ip(start_ip);
        Serial.println(string_stored);
        Serial.println("\n");
     
        Serial.print(F("End IP for scan"));
        Serial.println(string_format_ip);
        GetIP(end_ip);
        write_EEPROM(26, end_ip , sizeof(end_ip));
        Serial.println();
        print_ip(end_ip);
        Serial.println(string_stored);
        Serial.println("\n");
      } else {
        Serial.println(F("Use Subnetting"));
        Serial.println(string_stored);
        Serial.println("\n");
      }
        
      //TODO: Get free AVR-ID from Server?
      Serial.println(F("AVR-ID: "));
      GetString(AVRID, sizeof(AVRID));
      write_EEPROM(30, AVRID , sizeof(AVRID));
      Serial.print(AVRID);
      Serial.println(string_stored);
      Serial.println("\n");
  
      
      //Confirm settings and set configured = 1 in EEPROM
      write_EEPROM(0, 1);
      
      
      Serial.println("\n");
      Serial.println(F("Configuration finished"));
    
    } else {
      //Delete settings and set configured = 0 in EEPROM
      write_EEPROM(0, 0);
      Serial.println(F("Default configuration loaded"));
    }
    Serial.println("\n");
    
  } else {
    Serial.println(F("no configuration"));
  }
  
  
//_____________________Loading the values for the board__________________________
  if (read_EEPROM(0) != 1) {
    //use default values:
    Serial.println(F("No configuration stored yet. Using default values..."));
    Load_Default_Config();
  } else {
    //Read values from EEPROM:
    Serial.println(F("Using configuration from EEPROM."));
    read_EEPROM(1, mac_shield , sizeof(mac_shield));
    read_EEPROM(7, ip_shield , sizeof(ip_shield));
    read_EEPROM(11, gateway , sizeof(gateway));
    read_EEPROM(15, subnet , sizeof(subnet));
    useDhcp = read_EEPROM(19);
    pingrequest = read_EEPROM(20);
    useSubnetting = read_EEPROM(21);
    read_EEPROM(22, start_ip , sizeof(start_ip));
    read_EEPROM(26, end_ip , sizeof(end_ip));
    read_EEPROM(30, AVRID , sizeof(AVRID));
  }



//________________________Initialising of the board______________________________
  Serial.println(F("Try to get IP address from network..."));
  Serial.print(F(" MAC address of shield: "));
  print_mac(mac_shield);
  Serial.println();
  
  // Setup when no IP is known
  if (useDhcp == 0) {
    Ethernet.begin(mac_shield, ip_shield, dnsSrv, gateway, subnet);
    //Ethernet.begin(mac_shield, ip_shield);
  } else {
    if (Ethernet.begin(mac_shield) == 0) {
      Serial.println(F("DHCP failed, no automatic IP address assigned!"));
      Serial.print(F("Time for waiting for IP address: "));
      Serial.print(millis());
      Serial.println(F(" ms"));
      Serial.println(F("Trying to set manual IP address."));
      Ethernet.begin(mac_shield, ip_shield, dnsSrv, gateway, subnet);
      //Ethernet.begin(mac_shield, ip_shield);
    }
  }

  Serial.println(F(" Address assigned?"));
  Serial.print(" ");
  Serial.println(Ethernet.localIP());
  Serial.print(" ");
  Serial.println(Ethernet.subnetMask());
  Serial.print(" ");
  Serial.println(Ethernet.gatewayIP());
  Serial.println(F(" Setup complete\n"));
  Serial.print(F("Speicher: "));
  Serial.println(get_mem_unused());
  
  Serial.println(F("Starting server"));
  server.begin();
  

  //Set start_ip and end_ip if subnetting is choosed
  if (useSubnetting != 0) {
    for (byte i = 0; i < 4; i++) {
      readSubnet[i] = Ethernet.subnetMask()[i], DEC;
      readIP[i]     = Ethernet.localIP()[i], DEC;
      start_ip[i]   = readIP[i] & readSubnet[i];
      end_ip[i]     = readIP[i] | ~readSubnet[i];
      if (end_ip[i] == 255) {
        end_ip[i] = 254; 
      }
    }
    if (start_ip[3] == 0) {
        start_ip[3] = 1;   //TODO: Set to 2, because of Gateway, or don't scan gateway.
      }
  }

  Serial.print(F("\nStarting loop trough IP range "));
  print_ip(start_ip);
  Serial.print(F(" - "));
  print_ip(end_ip);
  
  Timer1.initialize(200000);
  Timer1.attachInterrupt(ServerListen);
  Serial.println("bla");
}

//___________________________Scan the network_________________________________
void loop() {
  Serial.print(F("Speicher (Loop-Start): "));
  Serial.println(get_mem_unused());
  for(int b = 0; b < 4; b++) { 
    currIP[b] = start_ip[b]; 
  }
  
  while (1) {
    if (currIP[1] <= end_ip[1]) {
      if (currIP[2] <= end_ip[2]) {
        if (currIP[3] <= end_ip[3]) {
          ICMPEchoReply echoReply = ping(currIP, pingrequest); 
          if (echoReply.status == SUCCESS) {
            // We found a device!
            Serial.print(F("Speicher (Device found): "));
            Serial.println(get_mem_unused());
            for(int mac = 0; mac < 6; mac++) {
              currMAC[mac] = echoReply.MACAddressSocket[mac];
            }
            
            Serial.print(F("Device found on: "));
            print_ip(currIP);
            Serial.print(F(" MAC: "));
            print_mac(currMAC);
            Serial.println();
            
          } else {
            // It's not responding, next one
            for(int mac = 0; mac < 6; mac++) {
              currMAC[mac] = 0;
            }
            Serial.print(F("No (pingable) device on IP "));
            print_ip(currIP);
            Serial.println();
          }
          send_info_to_server(currIP, currMAC, AVRID);
          
          //TODO: That isn't really good...
          //for(int x = 0; x < 1000; x++) {
            //ServerListen();
            //delay(1);
          //}
          
          currIP[3]++;  
        } else {
          currIP[3] = start_ip[3];
          currIP[2]++;
        }
      } else {
        currIP[2] = start_ip[2];
        currIP[1]++;
      }
    } else {
      break; // Exit Loop
    }
  }
  Serial.print(F("Speicher (End ServerSend): "));
  Serial.println(get_mem_unused());
  Serial.println(F("Restart loop"));
}


void ServerListen() {
//  Serial.println("a");
  //Serial.println(F("Servers listening..."));
  // listen for incoming clients
  EthernetClient serverClient = server.available();
  if (serverClient) {
//    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (serverClient.connected()) {
      if (serverClient.available()) {
        char c = serverClient.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          serverClient.println("HTTP/1.1 200 OK");
          serverClient.println("Content-Type: text/html");
          serverClient.println("Connection: close");  // the connection will be closed after completion of the response
          serverClient.println();
          serverClient.println("<!DOCTYPE HTML>");
          serverClient.println("<html>");
          serverClient.println("<head>");
          serverClient.println("	<title>Autarc-Lan-User-Stat - Enter Username</title>");
          serverClient.println("	<meta http-equiv='Content-Type' content='text/html; charset=UTF-8' />");
          serverClient.println("</head>");
          serverClient.println("<body>");
          serverClient.println("	Enter your name for this device:<br /><br />");
          serverClient.println("	<form action='http://lan-user.danit.de/' method='GET' accept-charset='UTF-8'>");
          serverClient.print("		<p>AVR-ID:<br><input name='id' type='text' size='30' value='");
          //TODO: Only send really inputed values!
          serverClient.print(AVRID[0]);
          serverClient.print(AVRID[1]);
          serverClient.print(AVRID[2]);
          serverClient.print(AVRID[3]);
          serverClient.print(AVRID[4]);
          serverClient.print(AVRID[5]);
          serverClient.println("' readonly></p>");
          serverClient.print("		<p>MAC of Device:<br><input name='mac' type='text' size='30' value='");
          serverClient.print(mac_shield[0], HEX);
          serverClient.print(":");
          serverClient.print(mac_shield[1], HEX);
          serverClient.print(":");
          serverClient.print(mac_shield[2], HEX);
          serverClient.print(":");
          serverClient.print(mac_shield[3], HEX);
          serverClient.print(":");
          serverClient.print(mac_shield[4], HEX);
          serverClient.print(":");
          serverClient.print(mac_shield[5], HEX);
          serverClient.println("' readonly></p>");
          serverClient.println("		<p>Username:<br><input name='user' type='text' size='30' maxlength='30'></p>");
          serverClient.println("		<input type='submit' name='cmdStore' value='Store'/>");
          serverClient.println("	</form>");
          serverClient.println("</body>");     
          serverClient.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
//    delay(1);
    // close the connection:
    serverClient.stop();
//    Serial.println("client disconnected");
  }
}
