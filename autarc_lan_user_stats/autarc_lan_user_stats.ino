#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
// See https://github.com/BlakeFoster/Arduino-Ping/
#include "ICMPPing.h"
#include "default_config.h"

#ifdef LOG_TO_SD_AND_SERIAL
  #define LOG_TO_SD
#endif

#ifdef LOG_TO_SD
  #include <SD.h>
#endif

//________________________Prototypes of functions______________________________
//Prototypes IP-functions
void print_message(String message);
String ip_to_string(byte ip[4]);
String mac_to_string(byte mac[6]);
void readSubnettingIP(void);
char tryDHCP(void);
void getNetworkName(void);
char compare_CharArray(char *char1, char *char2, char sizechar1, char sizechar2);
char connect_getNetworkName(EthernetClient &client);
void startConnection(void);
char renewDHCP(void);
void readConnectionValues(void);
void printConnectionDetails(void);
void send_info_to_server_troublehandler(char *name);
byte send_info_to_server_check_troubles(char *name);
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

//Prototypes global functions
#ifdef SHOW_MEMORY
  int freeRam(void);
#endif

#ifdef LOG_TO_SD
  void init_SD(bool reinit = 0);
  void start_SD_Log(void);
  void end_SD_Log(void);
  void set_next_log_filename(char *FileVar);
  File logfile;
  char SDfileName[11];
  char tmpfileName[11];
  bool SDfailed;

  #ifdef LOG_TO_SD_AND_SERIAL
    #define LOG_PRINT( ... ) Serial.print( __VA_ARGS__ ); start_SD_Log(); ((SDfailed == 0) ? logfile.print( __VA_ARGS__ ) : 0); end_SD_Log()
    #define LOG_PRINT_LN( ... ) Serial.println( __VA_ARGS__ ); start_SD_Log(); logfile.println( __VA_ARGS__ ); end_SD_Log()
  #else
    #define LOG_PRINT( ... ) start_SD_Log(); ((SDfailed == 0) ? logfile.print( __VA_ARGS__ ) : 0); end_SD_Log()
    #define LOG_PRINT_LN( ... ) start_SD_Log(); logfile.println( __VA_ARGS__ ); end_SD_Log()
  #endif
#else
  #define LOG_PRINT( ... ) Serial.print( __VA_ARGS__ )
  #define LOG_PRINT_LN( ... ) Serial.println( __VA_ARGS__ )
#endif



//________________________Global declarations______________________________
const char string_format_ip[] = ", format \"000.111.222.333\": ";
const String newline = String("\n");
byte tries = 0;
byte tries_getNetworkName = 0;
byte countOfflineDevices = 0;

//char serverURL[] = "lan-user.danit.de";
char serverURL[] = "kolchose.org";
//char serverPath[] = "";
char serverPath[] = "/autarc_lan_user_stats/03/\?";
#define SERVER_ADD_URI "/ping_result/add"
#define SERVER_STATS_URI "/stats/network/"
#define SERVER_DEVICE_STATS_URI1 "/network/"
#define SERVER_DEVICE_STATS_URI2 "/device/"
#define SERVER_DEVICE_STATS_URI3 "/info"
#define SERVER_GET_ID_URI "/networks/list"
#define SERVER_SET_NAME_URI "/device/name"

char VersionNR[] = "1.5";  //TODO: Automatically?
#define MAX_DEVICES_INFO 5

byte offlineIP[MAX_DEVICES_INFO][4];
unsigned long timeScanned[MAX_DEVICES_INFO];

byte currIP[4];
byte currMAC[6];
unsigned long timeDeviceFound;

// Ping library configuration
SOCKET pingSocket              = 0;
ICMPPing ping(pingSocket, (uint16_t)random(0, 255));

EthernetServer server(80);

//_________________________________Setup_____________________________________________
void setup() {
  int configuration;
  delay(1000);
  Serial.begin(115200);
  #ifdef LOG_TO_SD
    init_SD();
    #ifdef LOG_TO_SD_AND_SERIAL
      print_message(F("The complete log output will also be stored on SD"));
    #else
      //TODO! Check, why?!
      Serial.println(F("The complete log will be stored on SD, so you can't use the Serial monitor even not for configuration."));
    #endif
  #endif
  #ifdef SEND_HARDWARE_TO_SERVER
    print_message(F("Hardware devices like Gateway, DNS Server and Arduino will be send to the server"));
  #else
    print_message(F("Hardware devices like Gateway, DNS Server and Arduino won't be send to the server"));
  #endif
  
  #ifdef SHOW_MEMORY
    print_message(String(F("Free Arduino Memory in bytes (startup): ")) + freeRam());
  #endif

  
  //________________________Configuration of the board______________________________
  //Todo: Change to print_message() function
  LOG_PRINT(F("Press any key start configuration"));
  for (char i = 0; i < 1 and Serial.available() <= 0; i++) {
    delay(1000);
    //Todo: Change to print_message() function
    LOG_PRINT(".");
  }

  configuration = Serial.read();
  if (configuration >= 0) {
    print_message(F("Starting configuration"));
    startConfiguration();

  }
  else {
    print_message(F("no configuration"));
  }

  //_____________________Loading the values for the board__________________________
  if (read_EEPROM(0) != 1) {
    //use default values:
    print_message(F("No configuration stored yet. Using default values..."));
    Load_Default_Config();
  }
  else {
    //Read values from EEPROM:
    print_message(F("Using configuration from EEPROM."));
    read_EEPROM(1, mac_shield , sizeof(mac_shield));
    read_EEPROM(7, ip_shield , sizeof(ip_shield));
    read_EEPROM(11, gateway , sizeof(gateway));
    read_EEPROM(15, subnet , sizeof(subnet));
    useDhcp = read_EEPROM(19);
    pingrequest = read_EEPROM(20);
    useSubnetting = read_EEPROM(21);
    read_EEPROM(22, start_ip , sizeof(start_ip));
    read_EEPROM(26, end_ip , sizeof(end_ip));
    read_EEPROM(30, NetworkName , sizeof(NetworkName));
    read_EEPROM(40, dnsSrv , sizeof(dnsSrv));
    retryHost = read_EEPROM(44);
    read_EEPROM(45, NetworkPwd , sizeof(NetworkPwd));
  }


  //________________________Initialising of the board______________________________
  print_message(F("Try to get IP address from network..."));
  print_message(String(F(" MAC address of shield: ")) + mac_to_string(mac_shield));

  startConnection();
  printConnectionDetails();


  #if ARDUINO > 105
    print_message(String(F("Visit http:\/\/")) + ip_to_string(ip_shield) + String(F("/ with your browser to add a name to your device. Tell all users to do the same.")));
  #else
    print_message(String(F("Visit http://")) + ip_to_string(ip_shield) + String(F("/ with your browser to add a name to your device. Tell all users to do the same.")));
  #endif

  #if ARDUINO > 105
    print_message(String(F("Also check out http:\/\/")) + serverURL + serverPath + F(SERVER_STATS_URI) + NetworkName + String(F(" to see your stats online!")));
  #else
    print_message(F("Also check out http://") + serverURL + serverPath + F(SERVER_STATS_URI) + NetworkName + F(" to see your stats online!"));
  #endif
  
  print_message(F("Starting server"));
  server.begin();


  readSubnettingIP();

  print_message(newline + String(F("Starting loop trough IP range ")) + ip_to_string(start_ip) + String(F(" - ")) + ip_to_string(end_ip));
  
}

//___________________________Scan the network_________________________________
void loop() {
  char filterResult;

  #ifdef SHOW_MEMORY
    print_message(String(F("Free Arduino Memory in bytes (start loop): ")) + freeRam());
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
            print_message(String(F("Arduino on IP ")) + ip_to_string(currIP));
            
            #ifdef SEND_HARDWARE_TO_SERVER
              //Set currMAC = MAC of Arduino so that it will be send to the statistic server
              for (int copymac = 0; copymac < 6; copymac++) {
                currMAC[copymac] = mac_shield[copymac];
              }
              send_info_to_server_troublehandler("Arduino");
            #endif
          }
          else {
            //free, gateway or dnsSrv
            if (pingDevice() == 1) {
              timeDeviceFound = millis();
              if (filterResult == 0) {
                //free IP
                print_message(String(F("Device found on IP ")) + ip_to_string(currIP) + String(F(" MAC: ")) + mac_to_string(currMAC));
                
                send_info_to_server_troublehandler("");
              }
              else if (filterResult == 2) {
                //IP of gateway
                print_message(String(F("Gateway found on IP ")) + ip_to_string(currIP) + String(F(" MAC: ")) + mac_to_string(currMAC));
                #ifdef SEND_HARDWARE_TO_SERVER
                  send_info_to_server_troublehandler("Gateway");
                #endif
              }
              else if (filterResult == 4) {
                //IP of dnsSrv
                print_message(String(F("DNS-Server found on IP ")) + ip_to_string(currIP) + String(F(" MAC: ")) + mac_to_string(currMAC));
                #ifdef SEND_HARDWARE_TO_SERVER
                  send_info_to_server_troublehandler("DNS-Server");
                #endif
              }
              else if (filterResult == 6) {
                //IP of gateway & dnsSrv
                print_message(String(F("Gateway found on IP ")) + ip_to_string(currIP) + String(F(" MAC: ")) + mac_to_string(currMAC));
                #ifdef SEND_HARDWARE_TO_SERVER
                  send_info_to_server_troublehandler("Gateway");
                #endif
                ServerListenLoop(4);
                print_message(String(F("DNS-Server found on IP ")) + ip_to_string(currIP) + String(F(" MAC: ")) + mac_to_string(currMAC));
                #ifdef SEND_HARDWARE_TO_SERVER
                  send_info_to_server_troublehandler("DNS-Server");
                #endif
              }
            }
            else {
              print_message(String(F("No (pingable) device on IP ")) + ip_to_string(currIP));
              
              offlineIP[countOfflineDevices][0] = currIP[0];
              offlineIP[countOfflineDevices][1] = currIP[1];
              offlineIP[countOfflineDevices][2] = currIP[2];
              offlineIP[countOfflineDevices][3] = currIP[3];
              timeScanned[countOfflineDevices] = millis();
              countOfflineDevices++;
              if (countOfflineDevices == MAX_DEVICES_INFO) {
                //number of max offline devices reached -> send info to server
                print_message(F("Max offline devices reached. Sending devices to server."));
                send_info_to_server_troublehandler("");
                countOfflineDevices = 0;
              }
            }
          }
          //Device send to Server, so reset the Mac Adress that it can not appear twice
          for (int macreset = 0; macreset < 6; macreset++) {
            currMAC[macreset] = 0x00;
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
    print_message(String(F("Free Arduino Memory in bytes (end loop): ")) + freeRam());
  #endif
  print_message(F("Restart loop"));
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
// 30 - 35     | NetworkName   | 6
// 40 - 43     | dnsSrv        | 4
// 44          | retryHost     | 1
// 45 - 51     | NetworkPwd    | 7


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
    //Todo: Change to print_message() function
    LOG_PRINT(ch);
    if (ch == '\r') {
      //Todo: Change to print_message() function
      LOG_PRINT_LN(newline);
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

  print_message(F("Load default configuration (0 = no): "));
  if (GetNumber() == 0) {
    print_message(F("Easy configuration? (0 = no): "));
    easyConfig = GetNumber();

    print_message(F("MAC Board, format \"00:00:00:00:00:00\": "));
    GetMAC(mac_shield);
    print_message(newline + mac_to_string(mac_shield) + newline);

    if (easyConfig == 0) {
      print_message(F("Use DHCP (0 = no): "));
      useDhcp = GetNumber();
      if (useDhcp == 0) {
        print_message(F("Don't use DHCP"));
        manualIPConfig();
      }
      else {
        print_message(F("Use DHCP"));
        tryDHCP();
      }

      print_message(F("Use Subnetting (0 = no): "));
      useSubnetting = GetNumber();
      if (useSubnetting == 0) {
        print_message(F("Don't use Subnetting\n"));

        print_message(String(F("Start IP for scan")) + string_format_ip);
        GetIP(start_ip);
        print_message(newline + ip_to_string(start_ip) + newline);

        print_message(String(F("End IP for scan")) + string_format_ip);
        GetIP(end_ip);
        print_message(newline + ip_to_string(end_ip) + newline);
      }
      else {
        print_message(F("Use Subnetting\n"));
      }

      print_message(F("Number of ping-requests: "));
      pingrequest = GetNumber();
      print_message(String(F("Number of ping-requests: ")) + pingrequest);


      print_message(F("Number of server retries: "));
      retryHost = GetNumber();
      print_message(String(F("Number of server retries: ")) + retryHost);
      
    }
    else {
      tryDHCP();
      useSubnetting = 1;
      pingrequest = 1;
      retryHost = 2;
    }

    print_message(F("Register Arduino online? (0 = no): "));
    if (GetNumber() == 0) {
      print_message(F("Network Name (5 chars): "));
      GetString(NetworkName, sizeof(NetworkName));
      print_message(NetworkName);

      print_message(F("Network Password (6 chars): "));
      GetString(NetworkPwd, sizeof(NetworkPwd));
      print_message(NetworkPwd);
    }
    else {
      getNetworkName();
    }

    //Store settings and set configured = 1 in EEPROM
    write_EEPROM(7, ip_shield , sizeof(ip_shield));
    write_EEPROM(11, gateway , sizeof(gateway));
    write_EEPROM(15, subnet , sizeof(subnet));
    write_EEPROM(40, dnsSrv , sizeof(dnsSrv));
    write_EEPROM(1, mac_shield , sizeof(mac_shield));
    write_EEPROM(30, NetworkName , sizeof(NetworkName));
    write_EEPROM(19, useDhcp);
    write_EEPROM(21, useSubnetting);
    write_EEPROM(22, start_ip , sizeof(start_ip));
    write_EEPROM(26, end_ip , sizeof(end_ip));
    write_EEPROM(20, pingrequest);
    write_EEPROM(44, retryHost);

    write_EEPROM(45, NetworkPwd , sizeof(NetworkPwd));

    write_EEPROM(0, 1);

    print_message(F("\nAll values have been stored!"));
    print_message(F("Configuration finished\n"));

  }
  else {
    //Delete settings and set configured = 0 in EEPROM
    write_EEPROM(0, 0);
    print_message(F("Default configuration loaded\n"));
  }
}

void manualIPConfig(void) {
  print_message(String(F("IP Board")) + string_format_ip);
  GetIP(ip_shield);
  print_message(newline + ip_to_string(ip_shield) + newline);

  print_message(String(F("IP Gateway")) + string_format_ip);
  GetIP(gateway);
  print_message(newline + ip_to_string(gateway) + newline);

  print_message(String(F("Subnetmask")) + string_format_ip);
  GetIP(subnet);
  print_message(newline + ip_to_string(subnet) + newline);

  print_message(String(F("IP DNS-Server")) + string_format_ip);
  GetIP(dnsSrv);
  print_message(newline + ip_to_string(dnsSrv) + newline);
}


//_________________________________IP functions______________________________________
String ip_to_string(byte ip[4]) {
    return (ip[0] + String(".") + ip[1] + String(".") + ip[2] + String(".") + ip[3]);
}

String mac_to_string(byte mac[6]) {
  return (String(mac[0], HEX) + String(":") + String(mac[1], HEX) + String(":") + String(mac[2], HEX) + String(":") + String(mac[3], HEX) + String(":") + String(mac[4], HEX) + String(":") + String(mac[5], HEX));
}

void print_message(String message) {
  LOG_PRINT_LN(message);
      //Serial.println(message);
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
  print_message(F("Testing DHCP..."));
  if (Ethernet.begin(mac_shield) == 0) {
    print_message(F("  DHCP failed, no automatic IP address assigned!"));
    print_message(F("You have to configurate the connection settings manual:"));
    useDhcp = 0;
    manualIPConfig();
  }
  else {
    //DHCP possible
    print_message(F("DHCP successful"));
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
        print_message(String(F("Free Arduino Memory in bytes (device found): ")) + freeRam());
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

void getNetworkName(void) {
  startConnection();
  EthernetClient client;

  if (connect_getNetworkName(client) == 1) {
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
              if (compare_CharArray(varNameChar, "network_name", sizeof(varNameChar), sizeof("network_name")) == 1) {
                if (i < (sizeof(NetworkName) - 1)) {
                  NetworkName[i] = tmpc;
                  NetworkName[i + 1] = '\0';
                }
              }
              else if (compare_CharArray(varNameChar, "network_password", sizeof(varNameChar), sizeof("network_password")) == 1) {
                if (i < (sizeof(NetworkPwd) - 1)) {
                  NetworkPwd[i] = tmpc;
                  NetworkPwd[i + 1] = '\0';
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
      print_message(String(F("\n\n--------------\nYour account data:\n")) + NetworkName + newline + NetworkPwd + String(F("\n--------------\ndisconnecting.")));
      client.stop();
    }
  }
  else {
    //Connection to HTTP-Server failed
    print_message(F("Can't connect to HTTP-Server. Please try it later or restart the board."));
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

char connect_getNetworkName(EthernetClient &client) {
  if (client.connect(serverURL, 80) == 1) {
    print_message(String(F("Connected to HTTP Server")) + serverURL);

    // Make a HTTP request:
    client.print(F("GET "));
    client.print(serverPath);
    client.print(F(SERVER_GET_ID_URI)); // Just a dummy call?
    client.println(F(" HTTP/1.1"));
    client.print(F("Host: "));
    client.println(serverURL);
    client.print(F("User-Agent: Autarc_LAN_User_Stats "));
    client.println(VersionNR);
    client.println(F("Connection: close"));
    client.println(); // Important!

    print_message(String(client.status()));
    //client.stop();
    return 1;

  }
  else {
    print_message(F("NOT connected to HTTP Server\n"));
    print_message(String(client.status()));
    client.stop();
    if (tries_getNetworkName < 2) {
      tries_getNetworkName++;
      print_message(F("Retry to connect..."));
      connect_getNetworkName(client);
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
        print_message(F("DHCP failed, no automatic IP address assigned!"));
        print_message(F("Trying to reconnect in 20 seconds..."));
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
    print_message(F("DHCP renewed"));
    readConnectionValues();
    return 1;
  }
  else if (result == 4) {
    print_message(F("DHCP rebind"));
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
  print_message(String(F(" Address assigned?\n ")) + ip_to_string(ip_shield) + String("\n ") + ip_to_string(subnet) + String("\n ") + ip_to_string(gateway) + String(F("\nSetup complete\n")));
  #ifdef SHOW_MEMORY
    print_message(String(F("Free Arduino Memory in bytes (setup complete): ")) + freeRam());
  #endif
}

void send_info_to_server_troublehandler(char *name) { 
  while (send_info_to_server_check_troubles(name) == 0) {}
}

byte send_info_to_server_check_troubles(char *name) {
  if (send_info_to_server(name) == 0) {
    //Connection to HTTP-Server failed -> ping gateway
    print_message(F("Connection to HTTP-Server failed"));
    ICMPEchoReply echoReplyGateway = ping(gateway, pingrequest);
    if (echoReplyGateway.status == SUCCESS) {
      // Gateway response -> HTTP-Server offline?
      print_message(F("HTTP-Server may be broken. Trying again in 30 seconds."));
      ServerListenLoop(1); //30seconds
      return 0;
    }
    else {
      // Gateway also not available -> Connection problem -> Try to reconnect
      print_message(F("There is a connection error. Trying to start new connection..."));
      startConnection();
      printConnectionDetails();
      //Reconnected. Try again to send info
      return 0;
    }
  }
  else {
    //send_info_to_server succeded
    return 1;
  }
}


char send_info_to_server(char *name) {
  renewDHCP();

  EthernetClient client;
  #ifdef SHOW_MEMORY
    print_message(String(F("Free Arduino Memory in bytes (send info): ")) + freeRam());
  #endif
  if (client.connect(serverURL, 80) == 1) {
    tries = 0;
    unsigned long timeDifference;
    
    print_message(String(F("Connected to HTTP Server ")) + serverURL + serverPath + SERVER_ADD_URI);
    
    // Make a HTTP request:
    client.print(F("POST "));
    client.print(serverPath);
    client.print(F(SERVER_ADD_URI));
    client.println(F(" HTTP/1.1"));
    //client.println("/ HTTP/1.1");
    client.print(F("Host: "));
    client.println(serverURL);
    client.print(F("User-Agent: Autarc_LAN_User_Stats "));
    client.println(VersionNR);
    client.println(F("Connection: close"));
    // client.println(F("Content-Type: application/x-www-form-urlencoded;"));
    client.println(F("Content-Type: application/json; charset=UTF-8"));
    client.print(F("Content-Length: "));
    client.println("300"); //TODO: Maybe calculate this later..?
    client.println(); // Important!

    client.print("{");

    client.print(F("\"network_name\":"));
    client.print("\"");
    client.print(NetworkName);
    client.print("\",");
 
    client.print(F("\"online\":"));
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
      
      if ((millis() - timeDeviceFound) < 0) {
        //consider overflow of time since arduino runs (afer ~50 days)
        timeDifference = (millis() - timeDeviceFound + 4294967295 + 1) / 1000;
      }
      else {
        timeDifference = (millis() - timeDeviceFound) / 1000;
      }
      client.print(timeDifference);
      
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
    
    client.print(F("\"offline\":"));
    client.print("[");
    byte tmpSendOffline;
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
      
      if ((millis() - timeScanned[tmpSendOffline]) < 0) {
        //consider overflow of time since arduino runs (afer ~50 days)
        timeDifference = (millis() - timeScanned[tmpSendOffline] + 4294967295 + 1) / 1000;
      }
      else {
        timeDifference = (millis() - timeScanned[tmpSendOffline]) / 1000;
      }
      client.print(timeDifference);
      client.print("}");
    }
    client.print("]");
    countOfflineDevices = 0;
    
    client.println("}");
    
    print_message(String(F("Ethernet Client status: ")) + client.status()); // 23 Code together with HTTP Timeout: could mean no/wrong MAC for Shield is set!
    char tmpc;
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.connected())
    {
      if (client.available()) {
        tmpc = client.read();
        //Todo: Change to print_message() function
        LOG_PRINT(tmpc);  //prints the servers answer //TODO Check if it's empty
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
    print_message(F("NOT connected to HTTP Server\n\n"));
    print_message(String(client.status()));
    client.stop();
    if (tries < retryHost) {
      tries++;
      print_message(F("Retry to connect..."));
      ServerListenLoop(4);
      send_info_to_server(name);  //Todo: Maybe change this to save memory. Also we could get problems with the recursive function!
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
    print_message(F("new client"));
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (serverClient.connected()) {
      if (serverClient.available()) {
        
        char c = serverClient.read();
        //Serial.write(c);  //prints the clients request

      #ifdef LOG_TO_SD
        if (c == 'G') {
          c = serverClient.read();
          if (c == 'E') {
            c = serverClient.read();
            if (c == 'T') {
              c = serverClient.read();
              if (c == ' ') {
                c = serverClient.read();
               if (c == '/') {
                 c = serverClient.read();
                 if (c == ' ') {
                   //no sub file -> show actual logfile
                   tmpfileName[4] = SDfileName[4];
                   tmpfileName[5] = SDfileName[5];
                 }
                 else {
                  //now recieve the filename
                  tmpfileName[4] = c;
                  char c = serverClient.read();
                  tmpfileName[5] = c;
                  //log file NR stored in tmpfileName
                 }
                 print_message(String(F("requested logfile: ")) + tmpfileName);
                 c = serverClient.read();
               }
              }
            }
          }
        }
      #endif
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

          #if ARDUINO > 105
            serverClient.print(F("  <script src='http:\/\/"));
          #else
            serverClient.print(F("  <script src='http://"));
          #endif
          serverClient.println(F("code.jquery.com/jquery-1.10.1.min.js'></script>"));
          serverClient.println(F("<script type='text/javascript'>"));
          serverClient.println(F("$(document).ready(function() {"));
          serverClient.println(F("$('#go').on('click', function() {"));
          serverClient.print(F("var formData = '{\"network_name\": \""));
          serverClient.print(NetworkName);
          serverClient.print(F("\",\"network_password\": \""));
          serverClient.print(NetworkPwd);
          serverClient.print(F("\",\"mac\": \""));
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
          serverClient.println(F("\",\"name\": \"' + $('#name').val() + '\"}';"));
          serverClient.println(F("$.ajax({"));
          serverClient.println(F("  type: 'POST',"));
          
          #if ARDUINO > 105
            serverClient.print(F("  url: 'http:\/\/"));
          #else
            serverClient.print(F("  url: 'http://"));
          #endif
          
          serverClient.print(serverURL);
          serverClient.print(serverPath);
          serverClient.print(F(SERVER_SET_NAME_URI));
          serverClient.println(F("',"));
          serverClient.println(F("  data: formData,"));
          serverClient.println(F("  success: function(){},"));
          serverClient.println(F("  dataType: 'json'"));
          //Todo: Check, if Accept Header was changed at Stat-Server
          //serverClient.println(F("  dataType: 'json',"));
          //serverClient.println(F("  contentType: 'application/json; charset=UTF-8'"));
          serverClient.println(F("});"));
          serverClient.println(F("});"));
          serverClient.println(F("})"));
          serverClient.println(F("</script>"));          
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
          serverClient.print(F(SERVER_STATS_URI));
          serverClient.print(NetworkName);
          serverClient.println(F("'>Go to the network statistic</a><br /><br />"));
          #if ARDUINO > 105
            serverClient.print(F("    <a href='http:\/\/"));
          #else
            serverClient.print(F("    <a href='http://"));
          #endif
          serverClient.print(serverURL);
          serverClient.print(serverPath);
          serverClient.print(F(SERVER_DEVICE_STATS_URI1));
          serverClient.print(NetworkName);
          serverClient.print(F(SERVER_DEVICE_STATS_URI2));
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
          serverClient.print(F(SERVER_DEVICE_STATS_URI3));
          serverClient.println(F("'>Go to the device statistic</a><br /><br />"));
          
          serverClient.println(F("	</div>"));
          serverClient.println(F("	<div>"));
          serverClient.println(F("		<br /><br />"));
          serverClient.println(F("			<p>Enter a name for the device that is vistiting this page:<br><input id='name' type='text'></p>"));
          serverClient.print(F("			<p>Network Name:<br><input type='text' value='"));
          serverClient.print(NetworkName);
          serverClient.println(F("' readonly></p>"));
          serverClient.print(F("			<p>MAC of Device:<br><input type='text' value='"));
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
          serverClient.println(F("			<input type='button' id='go' value='Save device name'></input>"));
          serverClient.println(F("		</form>"));
          serverClient.println(F("	</div>"));

          #ifdef LOG_TO_SD
            serverClient.println(F("  <br />"));
            serverClient.println(F("  <br />"));
            serverClient.println(F("  <div>"));
            serverClient.print(F("    The current log file is <a href=\""));
            serverClient.print(SDfileName[4]);
            serverClient.print(SDfileName[5]);
            serverClient.print(F("\">"));
            serverClient.print(SDfileName);
            serverClient.println(F("</a>.<br />"));
            
//Todo: Maye enable SD  
            File fileToShow;
            fileToShow = SD.open(tmpfileName); //Load actual log file
            if (fileToShow) {
              serverClient.print(F("    The requested log file ("));
              serverClient.print(tmpfileName);
              serverClient.println(F(") contains the following log:"));
              serverClient.println(F("    <br />"));
              serverClient.println(F("    <textarea rows='50' cols='100'>"));
              while(fileToShow.available())
              {
                serverClient.write(fileToShow.read());  //send log file to client
              }
              fileToShow.close();
              serverClient.println(F("    </textarea>"));
            }
            else {
              serverClient.print(F("    The requested log file ("));
              serverClient.print(tmpfileName);
              serverClient.println(F(") doesn't exist!"));
            }
            
            serverClient.println(F("  </div><br />"));
            
            serverClient.println(F("  <div>"));
            serverClient.println(F("Other log-files on SD:<br />"));

            if (SD.exists("log/")) {
                //Print all existing log files
                tmpfileName[4] = '0';
                tmpfileName[5] = '0';
                int i;
                serverClient.println(F("    <ul>"));
                for (i=0;i<=99;i++) {
                  if (SD.exists(tmpfileName)) {
                    serverClient.print(F("      <li><a href=\""));
                    serverClient.print(tmpfileName[4]);
                    serverClient.print(tmpfileName[5]);
                    serverClient.print(F("\">"));
                    serverClient.print(tmpfileName);
                    serverClient.println(F("</a></li>"));
                  }
                  set_next_log_filename(tmpfileName);
                }
                serverClient.println(F("    </ul>"));
              }
              else {
                //no log directory
                serverClient.println(F("No logfiles found"));
              }
//Todo: Maybe disable SD      
            serverClient.println(F("  </div>"));
          #endif
          
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
    print_message(F("client disconnected"));
  }
}

#ifdef SHOW_MEMORY
  int freeRam(void) {
    extern int __heap_start, *__brkval; 
    int v; 
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
  }
#endif

#ifdef LOG_TO_SD
  void init_SD(bool reinit) {
    SDfileName[0] = 'l';
    SDfileName[1] = 'o';
    SDfileName[2] = 'g';
    SDfileName[3] = '/';
    SDfileName[4] = '0';
    SDfileName[5] = '0';
    SDfileName[6] = '.';
    SDfileName[7] = 't';
    SDfileName[8] = 'x';
    SDfileName[9] = 't';
    SDfileName[10] = '\0';

    tmpfileName[0] = SDfileName[0];
    tmpfileName[1] = SDfileName[1];
    tmpfileName[2] = SDfileName[2];
    tmpfileName[3] = SDfileName[3];
    tmpfileName[4] = SDfileName[4];
    tmpfileName[5] = SDfileName[5];
    tmpfileName[6] = SDfileName[6];
    tmpfileName[7] = SDfileName[7];
    tmpfileName[8] = SDfileName[8];
    tmpfileName[9] = SDfileName[9];
    tmpfileName[10] = SDfileName[10];
        
    pinMode(10, OUTPUT);
    pinMode(4, OUTPUT);
    
    //activate SD
    digitalWrite(10, HIGH);  //turn off ETH
    digitalWrite(4, LOW);
    
      if (!SD.begin(4)) {
        Serial.println(F("Initializing of SD card failed! No log will be stored!"));
        SDfailed = 1;
      }
      else {
        Serial.println(F("SD card initialized"));
      }

      if (SD.exists("log/")) {
        //Check for existing files and set the actual log file to the continued one
        // if all 99 files exist, not important which was the actual file always start with 00)
        int i;
        for (i=0;i<=99;i++) {
          if (SD.exists(SDfileName)) {
            set_next_log_filename(SDfileName);
          }
          else {
            break;
          }
        }
        //delete the new file, if it exist (only case if all 99 files exist; so delete 00)
        SD.remove(SDfileName);
        Serial.print(F("New logfile will be created: "));
        Serial.println(SDfileName);
      }
      else {
        SD.mkdir("log");
      }
      
      logfile = SD.open(SDfileName, FILE_WRITE);

      if (logfile) {
        SDfailed = 0;
        if (reinit == 0) {
          logfile.println(F("### Restart of board ###"));
        }
        else {
          logfile.println(F("--- Reinit of SD card ---"));
        }
        logfile.close();
      }
      else {
        SDfailed = 1;
        Serial.println(F("Can't write to SD card! No log will be stored!"));
      }
      
    if (reinit == 0) {
      //activate ETH
      digitalWrite(4, HIGH);  //turn off SD
      digitalWrite(10, LOW);
    }
  }

  void start_SD_Log(void) {
    //activate SD
    digitalWrite(10, HIGH);  //turn off ETH
    digitalWrite(4, LOW);

    if (SDfailed == 1) {
      init_SD(1);
    }
    if (SDfailed == 0) {
      logfile = SD.open(SDfileName, FILE_WRITE);
      
      if (logfile) {
        SDfailed = 0;
        if (logfile.size() >= MAX_LOG_SIZE) {
          //next time new file -> set filename
          set_next_log_filename(SDfileName);
          
          //delete the new file, if it exist
          SD.remove(SDfileName);
          Serial.print(F("New logfile will be created: "));
          Serial.println(SDfileName);
        }
      }
      else {
        Serial.println(F("Can't write to SD card! No log will be stored!"));
        //try next time to reinit the SD card
        SDfailed = 1;
      }
    }
  }

  void end_SD_Log(void) {
    if (logfile) {
      logfile.close();
    }
    //activate ETH
    digitalWrite(4, HIGH);  //turn off SD
    digitalWrite(10, LOW);  
  }

  void set_next_log_filename(char *FileVar) {
    int fileNr = (FileVar[5] - '0');
    if (fileNr == 9) {
      fileNr = (FileVar[4] - '0');
      if (fileNr == 9) {
        //99 files reached, start with 00
        FileVar[4] = '0';
      }
      else {
        fileNr = fileNr + 49;
        FileVar[4] = char(fileNr);
      }
      FileVar[5] = '0';
    }
    else {
      fileNr = fileNr + 49;
      FileVar[5] = char(fileNr);
    }
  }
#endif
    
