#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
// See https://github.com/BlakeFoster/Arduino-Ping/
#include "ICMPPing.h"
#include "IPHelper.h"
#include "memcheck.h"
#include "default_config.h"
//#include "TimerOne.h"
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
// 40 - 43     | dnsSrv        | 4
// 44          | retryHost     | 1
// 45 - 51     | AVRpsw        | 7


byte readSubnet[4];
byte readIP[4];

byte currIP[4];
byte currMAC[6];

byte easyConfig;
int configuration;

const char string_format_ip[] = ", format \"000.111.222.333\": ";

char serverURL[] = "lan-user.danit.de";

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
      Serial.println(F("Easy configuration? (0 = no): "));
      easyConfig = GetNumber();

      Serial.println(F("MAC Board, format \"00:00:00:00:00:00\": "));
      GetMAC(mac_shield);
      Serial.println();
      print_mac(mac_shield);
      Serial.println("\n");

      if (easyConfig == 0) {
        Serial.println(F("Use DHCP (0 = no): "));
        useDhcp = GetNumber();
        if (useDhcp == 0) {
          Serial.println(F("Don't use DHCP"));

          manualIPConfig();

        } 
        else {
          Serial.println(F("Use DHCP"));
          tryDHCP();
        }

        Serial.println(F("Use Subnetting (0 = no): "));
        useSubnetting = GetNumber();
        if (useSubnetting == 0) {
          Serial.println(F("Don't use Subnetting"));
          Serial.println("\n");

          Serial.print(F("Start IP for scan"));
          Serial.println(string_format_ip);
          GetIP(start_ip);
          Serial.println();
          print_ip(start_ip);
          Serial.println("\n");

          Serial.print(F("End IP for scan"));
          Serial.println(string_format_ip);
          GetIP(end_ip);
          Serial.println();
          print_ip(end_ip);
          Serial.println("\n");
        } 
        else {
          Serial.println(F("Use Subnetting"));
          Serial.println("\n");
        }

        Serial.println(F("Number of ping-requests: "));
        pingrequest = GetNumber();
        Serial.print(F("Number of ping-requests: "));
        Serial.print(pingrequest);
        Serial.println("\n");

        Serial.println(F("Number of server retries: "));
        retryHost = GetNumber();
        Serial.print(F("Number of server retries: "));
        Serial.print(retryHost);
        Serial.println(retryHost);
        Serial.println("\n");

      } 
      else {
        tryDHCP();
        useSubnetting = 1;
        pingrequest = 4;
        retryHost = 2;
      }

      Serial.println(F("Register AVR online? (0 = no): "));
      if (GetNumber() == 0) {
        Serial.println(F("AVR-ID (5 chars): "));
        GetString(AVRID, sizeof(AVRID));
        Serial.print(AVRID);
        Serial.println("\n");

        Serial.println(F("AVR Password (6 chars): "));
        GetString(AVRpsw, sizeof(AVRpsw));
        Serial.print(AVRpsw);
        Serial.println("\n");
      } 
      else {
        //TODO: What if connection fails?
        getAVRID();
      }



      //Store settings and set configured = 1 in EEPROM
      write_EEPROM(7, ip_shield , sizeof(ip_shield));
      write_EEPROM(11, gateway , sizeof(gateway));
      write_EEPROM(15, subnet , sizeof(subnet)); 
      write_EEPROM(40, dnsSrv , sizeof(dnsSrv));
      write_EEPROM(1, mac_shield , sizeof(mac_shield));
      write_EEPROM(30, AVRID , sizeof(AVRID));
      write_EEPROM(19, useDhcp);
      write_EEPROM(21, useSubnetting);
      write_EEPROM(22, start_ip , sizeof(start_ip));
      write_EEPROM(26, end_ip , sizeof(end_ip));
      write_EEPROM(20, pingrequest);
      write_EEPROM(44, retryHost);

      write_EEPROM(45, AVRpsw , sizeof(AVRpsw));

      write_EEPROM(0, 1);


      Serial.println("\n");
      Serial.println(F("All values have been stored!"));
      Serial.println(F("Configuration finished"));

    } 
    else {
      //Delete settings and set configured = 0 in EEPROM
      write_EEPROM(0, 0);
      Serial.println(F("Default configuration loaded"));
    }
    Serial.println("\n");

  } 
  else {
    Serial.println(F("no configuration"));
  }


  //_____________________Loading the values for the board__________________________
  if (read_EEPROM(0) != 1) {
    //use default values:
    Serial.println(F("No configuration stored yet. Using default values..."));
    Load_Default_Config();
  } 
  else {
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
    read_EEPROM(40, dnsSrv , sizeof(dnsSrv));
    retryHost = read_EEPROM(44);
    read_EEPROM(45, AVRpsw , sizeof(AVRpsw));
  }



  //________________________Initialising of the board______________________________
  Serial.println(F("Try to get IP address from network..."));
  Serial.print(F(" MAC address of shield: "));
  print_mac(mac_shield);
  Serial.println();

  startConnection();
  printConnectionDetails();
  

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

  //Timer1.initialize(200000);
  //Timer1.attachInterrupt(ServerListen);
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

          } 
          else {
            // It's not responding
            for(int mac = 0; mac < 6; mac++) {
              currMAC[mac] = 0;
            }
            Serial.print(F("No (pingable) device on IP "));
            print_ip(currIP);
            Serial.println();
          }
          
          send_info_to_server_troublehandler();

          //TODO: That isn't really good...
          for(int x = 0; x < 1000; x++) {
            ServerListen();
            delay(1);
          }

          currIP[3]++;  
        } 
        else {
          currIP[3] = start_ip[3];
          currIP[2]++;
        }
      } 
      else {
        currIP[2] = start_ip[2];
        currIP[1]++;
      }
    } 
    else {
      break; // Exit Loop
    }
  }
  Serial.print(F("Speicher (End ServerSend): "));
  Serial.println(get_mem_unused());
  Serial.println(F("Restart loop"));
}


void manualIPConfig() {
  Serial.print(F("IP Board"));
  Serial.println(string_format_ip);
  GetIP(ip_shield);
  Serial.println();
  print_ip(ip_shield);
  Serial.println("\n");

  Serial.print(F("IP Gateway"));
  Serial.println(string_format_ip);
  GetIP(gateway);
  Serial.println();
  print_ip(gateway);
  Serial.println("\n");

  //TODO: Check if it works fine!
  Serial.print(F("Subnetmask"));
  Serial.println(string_format_ip);
  GetIP(subnet);
  Serial.println();
  print_ip(subnet);
  Serial.println("\n");

  Serial.print(F("IP DNS-Server"));
  Serial.println(string_format_ip);
  GetIP(dnsSrv);
  Serial.println();
  print_ip(dnsSrv);
  Serial.println("\n");
}

char tryDHCP() {
  Serial.println(F("Testing DHCP..."));
  if (Ethernet.begin(mac_shield) == 0) {
    Serial.println(F("  DHCP failed, no automatic IP address assigned!"));
    Serial.print(F("  Time for waiting for IP address: "));
    Serial.print(millis());
    Serial.println(F(" ms"));
    Serial.println(F("You have to configurate the connection settings manual:"));
    useDhcp = 0;
    manualIPConfig();
  } 
  else {
    //TODO: Close Ethernet DHCP test
    //Ethernet.stop();
    //DHCP possible
    Serial.println(F("DHCP successful"));
    useDhcp = 1;
    for (byte i = 0; i < 4; i++) {
      ip_shield[i] = Ethernet.localIP()[i], DEC;
      gateway[i] = Ethernet.gatewayIP()[i], DEC;
      subnet[i] = Ethernet.subnetMask()[i], DEC;
      dnsSrv[i] = Ethernet.dnsServerIP()[i], DEC;
    }
  }
}

void getAVRID() {
  startConnection();
  EthernetClient client;

  if (client.connect(serverURL, 80) == 1) {
    Serial.println(F("Connected to HTTP Server"));

    // Make a HTTP request:
    // client.print("GET /autarc_lan_user_stats/"); // kolchose.org 
    client.print("GET /");
    client.print("?getAVR_ID=true");


    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(serverURL);
    client.println("User-Agent: Autarc_LAN_User_Stats"); // TODO: Add version
    //TODO: Check if necessaryS
    client.println("Connection: close");
    client.println(); // Important!

    Serial.println(client.status());
    //client.stop();

  } 
  else {
    Serial.println(F("NOT connected to HTTP Server"));
    Serial.println("\n");
    Serial.println(client.status());
    client.stop();
  }


  int i = 0;
  int varCount = 0;
  char tmpc;
  char save = 0;

  // if there are incoming bytes available 
  // from the server, read them and print them:
  while (client.connected())
  {
    if (client.available()) {
      tmpc = client.read();

      if (tmpc == -1) {
        break; 
      }
      else if (tmpc == '<') {
        save = 1;
        i = 0;
      }
      else if (tmpc == '>') {
        save = 0;
        varCount++;
      }
      else if (save == 1) {
        switch (varCount) {
        case 0:
          if (i < (sizeof(AVRID) - 1)) {
            AVRID[i] = tmpc;
            AVRID[i + 1] = '\0';
          }
          break;
        case 1:
          if (i < (sizeof(AVRpsw) - 1)) {
            AVRpsw[i] = tmpc;
            AVRpsw[i + 1] = '\0';
          }
          break;
        }
        i++;
      }
    }
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println("\n");
    Serial.println(F("Your account data: "));
    Serial.println(AVRID);
    Serial.println(AVRpsw);
    Serial.println(F("disconnecting."));
    client.stop();
  }
}

void startConnection() {
  if (useDhcp == 0) {
    Ethernet.begin(mac_shield, ip_shield, dnsSrv, gateway, subnet);
  } 
  else {
    if (Ethernet.begin(mac_shield) == 0) {
      //TODO: Check if it works fine
      int result = Ethernet.maintain();
      if (result == 2 || result == 4) {
        Serial.println(F("DHCP renewed")); 
      } else {
        Serial.println(F("DHCP failed, no automatic IP address assigned!"));
        Serial.println(F("Trying to reconnect in 30 seconds..."));
        delay(30000);
        startConnection();
      }
    }
  }
}

void printConnectionDetails() {
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
}

void send_info_to_server_troublehandler() {
  if (send_info_to_server(currIP, currMAC, AVRID, AVRpsw, retryHost, serverURL) == 0) {
    //Connection to HTTP-Server failed -> ping gateway
    Serial.println(F("Connection to HTTP-Server failed"));
    ICMPEchoReply echoReplyGateway = ping(gateway, pingrequest); 
    if (echoReplyGateway.status == SUCCESS) {
      // Gateway response -> HTTP-Server offline?
      Serial.println(F("HTTP-Server may be broken. Trying again in 30 seconds."));
      //TODO: Wait 30 seconds delay(30000);
      //TODO: That isn't really good...
      for(int x = 0; x < 30000; x++) {
            ServerListen();
            delay(1);
      }
      send_info_to_server_troublehandler(); 
    } 
    else {
      // Gateway also not available -> Connection problem -> Try to reconnect
      Serial.println(F("There is a connection error. Trying to start new connection..."));
      startConnection();
      printConnectionDetails();
      //Reconnected. Try again to send info
      send_info_to_server_troublehandler();
    }
  }
}          
          
          
void ServerListen() {
  //Serial.println(F("Servers listening..."));
  // listen for incoming clients
  EthernetClient serverClient = server.available();
  if (serverClient) {
    Serial.println("new client");
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
          serverClient.println("	<p>");
          serverClient.print("		<a href='http://");
          serverClient.print(serverURL);
          serverClient.println("/'>Go to the online-statistic</a><br /><br />");
          serverClient.println("	</p>");
          serverClient.println("	<p>");
          serverClient.println("		Enter your name for this device:<br /><br />");
          serverClient.print("		<form action='http://");
          serverClient.print(serverURL);
          serverClient.println("/' method='GET' accept-charset='UTF-8'>");
          serverClient.print("			<p>AVR-ID:<br><input name='id' type='text' size='30' value='");
          serverClient.print(AVRID);
          serverClient.println("' readonly></p>");
          serverClient.print("			<p>MAC of Device:<br><input name='mac' type='text' size='30' value='");
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
          serverClient.println("			<p>Username:<br><input name='user' type='text' size='30' maxlength='30'></p>");
          serverClient.println("			<input type='submit' name='cmdStore' value='Store'/>");
          serverClient.println("		</form>");
          serverClient.println("	</p>");
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
    delay(10);
    // close the connection:
    serverClient.stop();
    Serial.println(F("client disconnected"));
  }
}


