#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
// See https://github.com/BlakeFoster/Arduino-Ping/
#include "ICMPPing.h"
#include "default_config.h"

#ifdef SHOW_MEMORY
  #include "memcheck.h"
  // init_mem();  //neccessary?
#endif


//________________________Prototypes of functions______________________________
//Prototypes IP-functions
void print_ip(byte* ip);
void print_mac(byte* mac);
void readSubnettingIP(void);
char tryDHCP(void);
void getAVRID(void);
char compare_CharArray(char *char1, char *char2, char sizechar1, char sizechar2);
char connect_getAVRID(EthernetClient &client);
void startConnection(void);
char renewDHCP(void);
void readConnectionValues(void);
void printConnectionDetails(void);
void send_info_to_server_troublehandler(char *name);
char send_info_to_server(char *name);
void ServerListenLoop(int count);
void ServerListen(void);
char filterDevice(void);
char pingDevice(void);

//Prototypes configuration-functions
void write_EEPROM(int startstorage, byte *value, int valuesize);
void write_EEPROM(int startstorage, char *value, int valuesize);
void write_EEPROM(int startstorage, byte value);
void read_EEPROM(int startstorage, byte *value, int valuesize);
void read_EEPROM(int startstorage, char *value, int valuesize);
byte read_EEPROM(int startstorage);
void GetString(char *buf, int bufsize);
byte GetNumber(void);
void GetIP(byte *IP);
void GetMAC(byte *MAC);
void startConfiguration(void);
void manualIPConfig(void);


//________________________Global declarations______________________________
const char string_format_ip[] = ", format \"000.111.222.333\": ";
byte tries = 0;
byte tries_getAVRID = 0;
byte countOfflineDevices = 0;

// char serverURL[] = "lan-user.danit.de";
char serverURL[] = "kolchose.org";
char serverPath[] = "/autarc_lan_user_stats/03/?";
char VersionNR[] = "1.2";  //TODO: Automatically?
#define MAX_DEVICES_INFO 5

byte offlineIP[MAX_DEVICES_INFO][4];
unsigned long timeScanned[MAX_DEVICES_INFO];

byte currIP[4];
byte currMAC[6];

// Ping library configuration
SOCKET pingSocket              = 0;
ICMPPing ping(pingSocket, (uint16_t)random(0, 255));

EthernetServer server(80);

//_________________________________Setup_____________________________________________
void setup() {
  int configuration;
  delay(1000);
  Serial.begin(115200);
  #ifdef SHOW_MEMORY
    Serial.print(F("Free Arduino Memory in bytes (startup): "));
    Serial.println(get_mem_unused());
  #endif

  //________________________Configuration of the board______________________________
  Serial.print(F("Press any key start configuration"));
  for (char i = 0; i < 1 and Serial.available() <= 0; i++) {
    delay(1000);
    Serial.print(".");
  }

  configuration = Serial.read();
  if (configuration >= 0) {
    Serial.println(F("Starting configuration"));
    startConfiguration();

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


  #if ARDUINO > 105
    Serial.print(F("Visit http:\/\/"));
  #else
    Serial.print(F("Visit http://"));
  #endif
  print_ip(ip_shield);
  Serial.println(F("/ with your browser to add a name to your device. Tell all users to do the same."));
  #if ARDUINO > 105
    Serial.print(F("Also check out http:\/\/"));
  #else
    Serial.print(F("Also check out http://"));
  #endif
  Serial.print(serverURL);
  Serial.print(serverPath);
  Serial.print(F("/stats/network/")); // GET /stats/network/[network_name][/range]
  Serial.print(AVRID);
  Serial.println(F(" to see your stats online!"));
  
  
  Serial.println(F("Starting server"));
  server.begin();


  readSubnettingIP();

  Serial.print(F("\nStarting loop trough IP range "));
  print_ip(start_ip);
  Serial.print(F(" - "));
  print_ip(end_ip);
  Serial.println("\n");
}

//___________________________Scan the network_________________________________
void loop() {
  char filterResult;

  #ifdef SHOW_MEMORY
    Serial.print(F("Free Arduino Memory in bytes (start loop): "));
    Serial.println(get_mem_unused());
  #endif
  for (int b = 0; b < 4; b++) {
    currIP[b] = start_ip[b];
  }

  while (1) {
    if (currIP[1] <= end_ip[1]) {
      if (currIP[2] <= end_ip[2]) {
        if (currIP[3] <= end_ip[3]) {
          filterResult = filterDevice();
          if (filterResult == 1) {
            //IP of shield
            Serial.print(F("Arduino on IP "));
            print_ip(currIP);
            Serial.println();
            send_info_to_server_troublehandler("Arduino");
          }
          else {
            //free, gateway or dnsSrv
            if (pingDevice() == 1) {
              if (filterResult == 0) {
                //free IP
                Serial.print(F("Device found on IP "));
                print_ip(currIP);
                Serial.print(F(" MAC: "));
                print_mac(currMAC);
                Serial.println();
                send_info_to_server_troublehandler("");
              }
              else if (filterResult == 2) {
                //IP of gateway
                Serial.print(F("Gateway found on IP "));
                print_ip(currIP);
                Serial.print(F(" MAC: "));
                print_mac(currMAC);
                Serial.println();
                send_info_to_server_troublehandler("Gateway");
              }
              else if (filterResult == 4) {
                //IP of dnsSrv
                Serial.print(F("DNS-Server found on IP "));
                print_ip(currIP);
                Serial.print(F(" MAC: "));
                print_mac(currMAC);
                Serial.println();
                send_info_to_server_troublehandler("DNS-Server");
              }
              else if (filterResult == 6) {
                //IP of gateway & dnsSrv
                Serial.print(F("Gateway found on IP "));
                print_ip(currIP);
                Serial.print(F(" MAC: "));
                print_mac(currMAC);
                Serial.println();
                send_info_to_server_troublehandler("Gateway");
                ServerListenLoop(4);
                Serial.print(F("DNS-Server found on IP "));
                print_ip(currIP);
                Serial.print(F(" MAC: "));
                print_mac(currMAC);
                Serial.println();
                send_info_to_server_troublehandler("DNS-Server");
              }
            }
            else {
              Serial.print("No (pingable) device on IP ");
              print_ip(currIP);
              Serial.println();
              
              offlineIP[countOfflineDevices][0] = currIP[0];
              offlineIP[countOfflineDevices][1] = currIP[1];
              offlineIP[countOfflineDevices][2] = currIP[2];
              offlineIP[countOfflineDevices][3] = currIP[3];
              timeScanned[countOfflineDevices] = millis();
              countOfflineDevices++;
              if (countOfflineDevices == MAX_DEVICES_INFO) {
                //number of max offline devices reached -> send info to server
                Serial.println(F("Max offline devices reached. Sending devices to server."));
                send_info_to_server_troublehandler("");
                countOfflineDevices = 0;
              }
            }
          }
          ServerListenLoop(4);
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
  #ifdef SHOW_MEMORY
    Serial.print(F("Free Arduino Memory in bytes (end loop): "));
    Serial.println(get_mem_unused());
  #endif
  Serial.println(F("Restart loop"));
  readSubnettingIP();  //Important if Subnet of the board has changed
}


//_________________________________Configuration functions______________________________________

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


void write_EEPROM(int startstorage, byte *value, int valuesize) {
  for (int i = 0; i < valuesize; i++) {
    EEPROM.write(i + startstorage, value[i]);
  }
}

void write_EEPROM(int startstorage, char *value, int valuesize) {
  for (int i = 0; i < valuesize; i++) {
    EEPROM.write(i + startstorage, value[i]);
  }
}

void write_EEPROM(int startstorage, byte value) {
  EEPROM.write(startstorage, value);
}


void read_EEPROM(int startstorage, byte *value, int valuesize) {
  for (int i = 0; i < valuesize; i++) {
    value[i] = EEPROM.read(i + startstorage);
  }
}

void read_EEPROM(int startstorage, char *value, int valuesize) {
  for (int i = 0; i < valuesize; i++) {
    value[i] = EEPROM.read(i + startstorage);
  }
}

byte read_EEPROM(int startstorage) {
  return EEPROM.read(startstorage);
}


void GetString(char *buf, int bufsize) {
  int i;
  char ch;
  for (i = 0; ; ++i) {
    while (Serial.available() == 0); // wait for character to arrive
    ch = Serial.read();
    Serial.print(ch);
    if (ch == '\r') {
      Serial.println("\n");
      break;
    }
    else if (ch == '\n') {
      //Ignore new-line
      i--;
    }
    else if (ch == 8) {
      //Backspace
      if (i >= 1) {
        i = i - 2;
      }
    }
    else {
      if (i < bufsize - 1) {
        buf[i] = ch;
        buf[i + 1] = 0; // 0 string terminator
      }
    }
  }
}

byte GetNumber(void) {
  char input[2];
  GetString(input, sizeof(input));
  return atoi(input);
}

void GetIP(byte *IP) {
  char input[16];
  GetString(input, sizeof(input));

  char *i;
  IP[0] = atoi(strtok_r(input, ".", &i));
  IP[1] = atoi(strtok_r(NULL, ".", &i));
  IP[2] = atoi(strtok_r(NULL, ".", &i));
  IP[3] = atoi(strtok_r(NULL, ".", &i));
}

void GetMAC(byte *MAC) {
  char input[18];
  GetString(input, sizeof(input));

  char *i;
  MAC[0] = strtol(strtok_r(input, ":", &i), NULL, 16);
  MAC[1] = strtol(strtok_r(NULL, ":", &i), NULL, 16);
  MAC[2] = strtol(strtok_r(NULL, ":", &i), NULL, 16);
  MAC[3] = strtol(strtok_r(NULL, ":", &i), NULL, 16);
  MAC[4] = strtol(strtok_r(NULL, ":", &i), NULL, 16);
  MAC[5] = strtol(strtok_r(NULL, ":", &i), NULL, 16);
}

void startConfiguration(void) {
  byte easyConfig;

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
      pingrequest = 1;
      retryHost = 2;
    }

    Serial.println(F("Register AVR online? (0 = no): "));
    if (GetNumber() == 0) {
      Serial.println(F("AVR-ID (5 chars) (NEW: Network Name): "));
      GetString(AVRID, sizeof(AVRID));
      Serial.print(AVRID);
      Serial.println("\n");

      Serial.println(F("AVR Password (6 chars): "));
      GetString(AVRpsw, sizeof(AVRpsw));
      Serial.print(AVRpsw);
      Serial.println("\n");
    }
    else {
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

void manualIPConfig(void) {
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


//_________________________________IP functions______________________________________
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

void readSubnettingIP(void) {
  //Set start_ip and end_ip if subnetting is choosed
  if (useSubnetting != 0) {
    for (byte i = 0; i < 4; i++) {
      start_ip[i]   = ip_shield[i] & subnet[i];
      end_ip[i]     = ip_shield[i] | ~subnet[i];
      if (end_ip[i] == 255) {
        end_ip[i] = 254;
      }
    }
    if (start_ip[3] == 0) {
      start_ip[3] = 1;
    }
  }
}

char tryDHCP(void) {
  Serial.println(F("Testing DHCP..."));
  if (Ethernet.begin(mac_shield) == 0) {
    Serial.println(F("  DHCP failed, no automatic IP address assigned!"));
    Serial.println(F("You have to configurate the connection settings manual:"));
    useDhcp = 0;
    manualIPConfig();
  }
  else {
    //DHCP possible
    Serial.println(F("DHCP successful"));
    useDhcp = 1;
    readConnectionValues();
  }
}

char filterDevice(void) {
  char check_shield = 0;
  char check_gateway = 0;
  char check_dnsSrv = 0;

  for (char i = 3; i >= 0; i--) {
    if (currIP[i] == ip_shield[i]) {
      check_shield++;
      if (currIP[i] == gateway[i]) {
        check_gateway++;
      }
      if (currIP[i] == dnsSrv[i]) {
        check_dnsSrv++;
      }
    }
    else if (currIP[i] == gateway[i]) {
      check_gateway++;
      if (currIP[i] == dnsSrv[i]) {
        check_dnsSrv++;
      }
    }
    else if (currIP[i] == dnsSrv[i]) {
      check_dnsSrv++;
    }
    else {
      return 0;
      break;
    }
  }

  char check_result = 0;
  if (check_shield == 4) {
    check_result++;
  }
  else {
    if (check_gateway == 4) {
      check_result += 2;
    }
    if (check_dnsSrv == 4) {
      check_result += 4;
    }
  }
  return check_result;
}

char pingDevice(void) {
  ICMPEchoReply echoReply = ping(currIP, pingrequest);
  if (echoReply.status == SUCCESS) {
    // We found a device!
    #ifdef SHOW_MEMORY
        Serial.print(F("Free Arduino Memory in bytes (device found): "));
        Serial.println(get_mem_unused());
    #endif
    for (int mac = 0; mac < 6; mac++) {
      currMAC[mac] = echoReply.MACAddressSocket[mac];
    }
    return 1;
  }
  else {
    // It's not responding
    return 0;
  }
}

void getAVRID(void) {
  startConnection();
  EthernetClient client;

  if (connect_getAVRID(client) == 1) {
    int i = 0;
    char tmpc;
    char startJSON = 0;
    char varNameChar[7];
    char startName = 0;
    char startVarValue = 0;

    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.connected())
    {
      if (client.available()) {
        tmpc = client.read();

        if (tmpc == -1) {
          break;
        }
        else if (startJSON == 0 && tmpc == '{') {
          startJSON = 1;
        }
        else if (startJSON == 1 && tmpc == '}') {
          startJSON = 0;
        }
        else if (startJSON == 1) {
          if (tmpc == '"') {
            if (startName == 0) {
              startName = 1;
              i = 0;
            }
            else {
              startName = 0;
            }
          }
          else if (startName == 0 && tmpc == ':') {
            startVarValue = 1;
          }
          else if (startName == 0 && tmpc == ',') {
            startVarValue = 0;
          }
          else if (startName == 1) {
            if (startVarValue == 0) {
              if (i < (sizeof(varNameChar) - 1)) {
                varNameChar[i] = tmpc;
                varNameChar[i + 1] = '\0';
                i++;
              }
            }
            else {
              if (compare_CharArray(varNameChar, "AVRID", sizeof(varNameChar), sizeof("AVRID")) == 1) {
                if (i < (sizeof(AVRID) - 1)) {
                  AVRID[i] = tmpc;
                  AVRID[i + 1] = '\0';
                }
              }
              else if (compare_CharArray(varNameChar, "AVRpsw", sizeof(varNameChar), sizeof("AVRpsw")) == 1) {
                if (i < (sizeof(AVRpsw) - 1)) {
                  AVRpsw[i] = tmpc;
                  AVRpsw[i + 1] = '\0';
                }
              }
              //Attention: If you add a var > 6 chars, you have to change the space of the char array!
              i++;
            }
          }
        }
      }
    }

    // if the server is disconnected, stop the client:
    if (!client.connected()) {
      Serial.println("\n");
      Serial.println(F("--------------"));
      Serial.println(F("Your account data: "));
      Serial.println(AVRID);
      Serial.println(AVRpsw);
      Serial.println(F("--------------"));
      Serial.println(F("disconnecting."));
      client.stop();
    }
  }
  else {
    //Connection to HTTP-Server failed
    Serial.println(F("Can't connect to HTTP-Server. Please try it later or restart the board."));
    while (1) {
    }
  }
}

char compare_CharArray(char *char1, char *char2, char sizechar1, char sizechar2) {
  for (byte i = 0; ; i++) {
    if (i >= sizechar1 || i >= sizechar2) {
      return 0;
    }
    else {
      if (char1[i] != char2[i]) {
        return 0;
      }
      if (char1[i] == '\0') {
        break;
      }
      else if (char2[i] == '\0') {
        break;
      }
    }
  }
  return 1;
}

char connect_getAVRID(EthernetClient &client) {
  if (client.connect(serverURL, 80) == 1) {
    Serial.print(F("Connected to HTTP Server"));
    Serial.println(serverURL);

    // Make a HTTP request:
    client.print(F("GET "));
    client.print(serverPath);
    client.print(F("/networks/list")); // Just a dummy call?
    client.println(F(" HTTP/1.1"));
    client.print(F("Host: "));
    client.println(serverURL);
    client.print(F("User-Agent: Autarc_LAN_User_Stats "));
    client.println(VersionNR);
    client.println(F("Connection: close"));
    client.println(); // Important!

    Serial.println(client.status());
    //client.stop();
    return 1;

  }
  else {
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
    readConnectionValues();
  }
  else {
    if (Ethernet.begin(mac_shield) == 0) {
      if (renewDHCP() == 0) {
        Serial.println(F("DHCP failed, no automatic IP address assigned!"));
        Serial.println(F("Trying to reconnect in 20 seconds..."));
        delay(20000);
        startConnection();
      }
    }
    else {
      readConnectionValues();
    }
  }
}

char renewDHCP(void) {
  delay(50);
  int result = Ethernet.maintain();
  delay(150);
  if (result == 2) {
    Serial.println(F("DHCP renewed"));
    readConnectionValues();
    return 1;
  }
  else if (result == 4) {
    Serial.println(F("DHCP rebind"));
    return 1;
  }
  else {
    return 0;
  }
}

void readConnectionValues(void) {
  for (byte i = 0; i < 4; i++) {
    ip_shield[i] = Ethernet.localIP()[i], DEC;
    gateway[i] = Ethernet.gatewayIP()[i], DEC;
    subnet[i] = Ethernet.subnetMask()[i], DEC;
    dnsSrv[i] = Ethernet.dnsServerIP()[i], DEC;
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
  Serial.println(F("Setup complete\n"));
  #ifdef SHOW_MEMORY
    Serial.print(F("Free Arduino Memory in bytes (setup complete): "));
    Serial.println(get_mem_unused());
  #endif
}

void send_info_to_server_troublehandler(char *name) {
  if (send_info_to_server(name) == 0) {
    //Connection to HTTP-Server failed -> ping gateway
    Serial.println(F("Connection to HTTP-Server failed"));
    ICMPEchoReply echoReplyGateway = ping(gateway, pingrequest);
    if (echoReplyGateway.status == SUCCESS) {
      // Gateway response -> HTTP-Server offline?
      Serial.println(F("HTTP-Server may be broken. Trying again in 30 seconds."));
      ServerListenLoop(1); //30seconds

      send_info_to_server_troublehandler(name);
    }
    else {
      // Gateway also not available -> Connection problem -> Try to reconnect
      Serial.println(F("There is a connection error. Trying to start new connection..."));
      startConnection();
      printConnectionDetails();
      //Reconnected. Try again to send info
      send_info_to_server_troublehandler(name);
    }
  }
}

char send_info_to_server(char *name) {
  renewDHCP();

  EthernetClient client;
  #ifdef SHOW_MEMORY
    Serial.print(F("Free Arduino Memory in bytes (send info): "));
    Serial.println(get_mem_unused());
  #endif
  if (client.connect(serverURL, 80) == 1) {
    tries = 0;
    Serial.print(F("Connected to HTTP Server "));
    Serial.println(serverURL);
    
    
    // Make a HTTP request:
    client.print(serverPath);
    client.println(F("/ping_result/add HTTP/1.1"));
    client.print(F("Host: "));
    client.println(serverURL);
    client.print(F("User-Agent: Autarc_LAN_User_Stats "));
    client.println(VersionNR);
    client.println(F("Content-Length: 300"));  //TODO: Maybe calculate this later..?
    client.println(F("Connection: close"));
    client.println(F("Content-Type: application/x-www-form-urlencoded"));
    client.println(); // Important!
    client.println(); // Important!

    client.print("{");

    client.print(F("\"network_name:\""));
    client.print("\"");
    client.print(AVRID);
    client.print("\",");
 
    client.print(F("\"online:\""));
    client.print("[");
    if (countOfflineDevices != MAX_DEVICES_INFO) {
      client.print("{");
      client.print(F("\"ip\":"));
      client.print("\"");
      client.print(currIP[0]);
      client.print(".");
      client.print(currIP[1]);
      client.print(".");
      client.print(currIP[2]);
      client.print(".");
      client.print(currIP[3]);
      client.print("\",");
      
      client.print(F("\"t\":"));
      client.print("0");
      client.print(",");
      
      client.print(F("\"mac\":"));
      client.print("\"");
      client.print(currMAC[0], HEX);
      client.print(":");
      client.print(currMAC[1], HEX);
      client.print(":");
      client.print(currMAC[2], HEX);
      client.print(":");
      client.print(currMAC[3], HEX);
      client.print(":");
      client.print(currMAC[4], HEX);
      client.print(":");
      client.print(currMAC[5], HEX);
      client.print("\"");
      
      client.print("}");
    }
    client.print("],");
    
    client.print(F("\"offline:\""));
    client.print("[");
    byte tmpSendOffline;
    long timeDifference;
    for (tmpSendOffline = 0; tmpSendOffline < countOfflineDevices; tmpSendOffline++) {
      if (tmpSendOffline != 0) {
        client.print(",");
      }
      client.print("{");
      client.print(F("\"ip\":"));
      client.print("\"");
      client.print(offlineIP[tmpSendOffline][0]);
      client.print(".");
      client.print(offlineIP[tmpSendOffline][1]);
      client.print(".");
      client.print(offlineIP[tmpSendOffline][2]);
      client.print(".");
      client.print(offlineIP[tmpSendOffline][3]);
      client.print("\",");
      client.print(F("\"t\":"));
      
      timeDifference = (millis() - timeScanned[tmpSendOffline]) / 1000;
      if (timeDifference < 0) {
        //overflow of time since arduino runs (afer ~50 days) - set time difference to 0. This shouldn't happen that often... ;)
        timeDifference = 0;
      }
      
      client.print(timeDifference);
      client.print("0");  //TODO: Set and count time!
      client.print("}");
    }
    client.print("]");
    countOfflineDevices = 0;
    
    client.print("}");

    Serial.print(F("Ethernet Client status: "));
    Serial.println(client.status()); // 23 Code together with HTTP Timeout: could mean no/wrong MAC for Shield is set!
    char tmpc;
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.connected())
    {
      if (client.available()) {
        tmpc = client.read();
        Serial.print(tmpc);  //prints the servers answer //TODO Check if it's empty
        if (tmpc == -1) {
          break;
        }
      }
    }
    // if the server disconnects, stop the client:
    if (!client.connected()) {
      client.stop();
    }
    delay(10);
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
      ServerListenLoop(4);
      send_info_to_server(name);
    }
    else {
      //Connection to server failed
      tries = 0;
      return 0;
    }
  }
}

void ServerListenLoop(int count) {
  //Not really nice, but it works...
  for (int i = 0; i < count; i++) {
    for (int x = 0; x < 1000; x++) {
      ServerListen();
      delay(1);
    }
  }
}

void ServerListen(void) {
  // listen for incoming clients
  EthernetClient serverClient = server.available();
  if (serverClient) {
    Serial.println(F("new client"));
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (serverClient.connected()) {
      if (serverClient.available()) {
        
        char c = serverClient.read();
        //Serial.write(c);  //prints the clients request

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          //Read the MAC of this device
          byte MACClient[6];
          W5100.readSnDHAR(0, MACClient);
          
          // send a standard http response header
          serverClient.println(F("HTTP/1.1 200 OK"));
          serverClient.println(F("Content-Type: text/html"));
          serverClient.println(F("Connection: close"));  // the connection will be closed after completion of the response
          serverClient.println(); // Important!
          serverClient.println(F("<!DOCTYPE HTML>"));
          serverClient.println(F("<html>"));
          serverClient.println(F("<head>"));
          serverClient.println(F("	<title>Autarc-Lan-User-Stat - Enter device name</title>"));
          serverClient.println(F("	<meta http-equiv='Content-Type' content='text/html; charset=UTF-8' />"));
          serverClient.println(F("</head>"));
          serverClient.println(F("<body>"));
          serverClient.println(F("	<div>"));
          #if ARDUINO > 105
            serverClient.print(F("		<a href='http:\/\/"));
          #else
            serverClient.print(F("		<a href='http://"));
          #endif
          serverClient.print(serverURL);
          serverClient.print(serverPath);
          serverClient.print(F("/stats/network/"));
          serverClient.print(AVRID);
          serverClient.println(F("'>Go to the usage statistics</a><br /><br />"));
          serverClient.println(F("	</div>"));
          serverClient.println(F("	<div>"));
          serverClient.println(F("		<br /><br />"));
          #if ARDUINO > 105
            serverClient.print(F("		<form action='http:\/\/"));
          #else
            serverClient.print(F("		<form action='http://"));
          #endif
          serverClient.print(serverURL);
          serverClient.println(F("/' method='POST' accept-charset='UTF-8'>"));
          serverClient.println(F("			<p>Enter a name for the device that is vistiting this page:<br><input name='user' type='text'></p>"));
          serverClient.print(F("			<p>AVR-ID:<br><input name='id' type='text' value='"));
          serverClient.print(AVRID);
          serverClient.println(F("' readonly></p>"));
          serverClient.print(F("			<input name='psw' type='hidden' value='"));
          serverClient.print(AVRpsw);
          serverClient.println("'>");
          serverClient.print(F("			<p>MAC of Device:<br><input name='mac' type='text' value='"));
          serverClient.print(MACClient[0], HEX);
          serverClient.print(":");
          serverClient.print(MACClient[1], HEX);
          serverClient.print(":");
          serverClient.print(MACClient[2], HEX);
          serverClient.print(":");
          serverClient.print(MACClient[3], HEX);
          serverClient.print(":");
          serverClient.print(MACClient[4], HEX);
          serverClient.print(":");
          serverClient.print(MACClient[5], HEX);
          serverClient.println(F("' readonly>"));
          #if ARDUINO > 105
            serverClient.print(F(" <a href='http:\/\/"));
          #else
            serverClient.print(F(" <a href='http://"));
          #endif
          serverClient.print(F("hwaddress.com?q="));
          serverClient.print(MACClient[0], HEX);
          serverClient.print(":");
          serverClient.print(MACClient[1], HEX);
          serverClient.print(":");
          serverClient.print(MACClient[2], HEX);
          serverClient.println(F("' target='_blank'>Vendor?</a></p>"));
          serverClient.println(F("			<input type='submit' name='cmdStore' value='Save device name'/>"));
          serverClient.println(F("		</form>"));
          serverClient.println(F("	</div>"));
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
