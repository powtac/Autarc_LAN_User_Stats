#include "IPHelper.h"

char tries = 0;
char tries_getAVRID = 0;


void print_ip(byte* ip) {
  Serial.print(ip[0]);
  Serial.print(".");
  Serial.print(ip[1]);
  Serial.print(".");
  Serial.print(ip[2]);
  Serial.print(".");
  Serial.print(ip[3]);
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
  Serial.print(mac[5], HEX);
}




char tryDHCP(void) {
  Serial.println(F("Testing DHCP..."));
  if (Ethernet.begin(mac_shield) == 0) {
    Serial.println(F("  DHCP failed, no automatic IP address assigned!"));
    Serial.println(F("You have to configurate the connection settings manual:"));
    useDhcp = 0;
    //TODO: ADD
    //manualIPConfig();
  } 
  else {
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

void getAVRID(void) {
  startConnection();
  EthernetClient client;
  
  if (connect_getAVRID(client) == 1) {
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
  } else {
    //Connection to HTTP-Server failed
    Serial.println(F("Can't connect to HTTP-Server. Please try it later or restart the board."));
    while (1) { 
    }
  }
}

char connect_getAVRID(EthernetClient &client) {
  if (client.connect(serverURL, 80) == 1) {
    Serial.println(F("Connected to HTTP Server"));

    // Make a HTTP request:
    // client.print(F("GET /autarc_lan_user_stats/")); // kolchose.org 
    client.print(F("GET /"));
    client.print(F("?getAVR_ID=true"));


    client.println(F(" HTTP/1.1"));
    client.print(F("Host: "));
    client.println(serverURL);
    client.print(F("User-Agent: Autarc_LAN_User_Stats"));
    client.println(VersionNR);
    client.println(F("Connection: close"));
    client.println(); // Important!

    Serial.println(client.status());
    //client.stop();
    return 1;
    
  } else {
    Serial.println(F("NOT connected to HTTP Server"));
    Serial.println("\n");
    Serial.println(client.status());
    client.stop();
    if (tries_getAVRID < 2) {
      tries_getAVRID++;
      Serial.println(F("Retry to connect..."));
      connect_getAVRID(client);
    } 
    else {
      //Connection to server failed
      return 0;
    }
  } 
}

void startConnection(void) {
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

void printConnectionDetails(void) {
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

void send_info_to_server_troublehandler(void) {
  if (send_info_to_server(currIP, currMAC) == 0) {
    //Connection to HTTP-Server failed -> ping gateway
    Serial.println(F("Connection to HTTP-Server failed"));
    ICMPEchoReply echoReplyGateway = ping(gateway, pingrequest); 
    if (echoReplyGateway.status == SUCCESS) {
      // Gateway response -> HTTP-Server offline?
      Serial.println(F("HTTP-Server may be broken. Trying again in 30 seconds."));
      //TODO: Wait 30 seconds delay(30000);
      ServerListenLoop(30); //30seconds
      
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

char send_info_to_server(byte* IP, byte* MAC) {
  // int result = Ethernet.maintain(); // renew DHCP
  // Serial.print(" DHCP renew:");
  // Serial.println(result); 
  // delay(1000);
  Serial.print(F("Speicher (send_info): "));
  Serial.println(get_mem_unused());
  EthernetClient client;

  if (client.connect(serverURL, 80) == 1) {
    tries = 0;
    Serial.println(F("Connected to HTTP Server"));

    // Make a HTTP request:
    // client.print(F("GET /autarc_lan_user_stats/")); // kolchose.org 
    client.print(F("GET /"));
    client.print(F("?AVR_ID="));
    client.print(AVRID);
    Serial.println(AVRID);
    client.print(F("&AVR_PSW="));
    client.print(AVRpsw);

    // HTTP String:  AVR_ID=AVR_ID&AVR_PSW=AVRpsw&IP[]=Ip&MAC[]=Mac&IP[]=IP&MAC[]=Mac
    client.print(F("&IP[]="));
    client.print(IP[0]);
    client.print(".");
    client.print(IP[1]);
    client.print(".");
    client.print(IP[2]);
    client.print(".");
    client.print(IP[3]);
    client.print(F("&MAC[]="));
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

    client.println(F(" HTTP/1.1"));
    client.print(F("Host: "));
    client.println(serverURL);
    client.print(F("User-Agent: Autarc_LAN_User_Stats"));
    client.println(VersionNR);
    client.println(F("Connection: close"));
    client.println(); // Important!

    Serial.println(client.status());
    client.stop();
    return 1;
  } 
  else {
    Serial.println(F("NOT connected to HTTP Server"));
    Serial.println("\n");
    Serial.println(client.status());
    client.stop();
    if (tries < retryHost) {
      tries++;
      Serial.println(F("Retry to connect..."));
      //TODO: Add this:
      //ServerListenLoop(4);
      send_info_to_server(IP, MAC);
    } 
    else {
      //Connection to server failed
      tries = 0;
      return 0;
    }
  }
}


void ServerListenLoop(int count) {
  //TODO: That isn't really good...
  for (int i = 0; i < count; i++) {
    for(int x = 0; x < 1000; x++) {
      ServerListen();
      delay(1);
    }
  }
}  
          
void ServerListen(void) {
  //Serial.println(F("Servers listening..."));
  // listen for incoming clients
  EthernetClient serverClient = server.available();
  if (serverClient) {
    Serial.println(F("new client"));
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
          serverClient.println(F("HTTP/1.1 200 OK"));
          serverClient.println(F("Content-Type: text/html"));
          serverClient.println(F("Connection: close"));  // the connection will be closed after completion of the response
          serverClient.println();
          serverClient.println(F("<!DOCTYPE HTML>"));
          serverClient.println(F("<html>"));
          serverClient.println(F("<head>"));
          serverClient.println(F("	<title>Autarc-Lan-User-Stat - Enter Username</title>"));
          serverClient.println(F("	<meta http-equiv='Content-Type' content='text/html; charset=UTF-8' />"));
          serverClient.println(F("</head>"));
          serverClient.println(F("<body>"));
          serverClient.println(F("	<p>"));
          serverClient.print(F("		<a href='http://"));
          serverClient.print(serverURL);
          serverClient.println(F("/'>Go to the online-statistic</a><br /><br />"));
          serverClient.println(F("	</p>"));
          serverClient.println(F("	<p>"));
          serverClient.println(F("		Enter your name for this device:<br /><br />"));
          serverClient.print(F("		<form action='http://"));
          serverClient.print(serverURL);
          serverClient.println(F("/' method='GET' accept-charset='UTF-8'>"));
          serverClient.print(F("			<p>AVR-ID:<br><input name='id' type='text' size='30' value='"));
          serverClient.print(AVRID);
          serverClient.println(F("' readonly></p>"));
          serverClient.print(F("			<p>MAC of Device:<br><input name='mac' type='text' size='30' value='"));
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
          serverClient.println(F("' readonly></p>"));
          serverClient.println(F("			<p>Username:<br><input name='user' type='text' size='30' maxlength='30'></p>"));
          serverClient.println(F("			<input type='submit' name='cmdStore' value='Store'/>"));
          serverClient.println(F("		</form>"));
          serverClient.println(F("	</p>"));
          serverClient.println(F("</body>"));     
          serverClient.println(F("</html>"));
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
    delay(20);
    // close the connection:
    serverClient.stop();
    Serial.println(F("client disconnected"));
  }
}
