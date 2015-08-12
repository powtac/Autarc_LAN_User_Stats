#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
// See https://github.com/BlakeFoster/Arduino-Ping/
#include "ICMPPing.h"
#include "default_config.h"

#ifdef LOG_TO_SD
  #include <SD.h>
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
  void init_SD(void);
  void start_SD_Log(void);
  void end_SD_Log(void);
  void set_next_log_filename(void);
  File logfile;
  char SDfileName[11];

  #ifdef LOG_TO_SD_AND_SERIAL
    #define LOG_PRINT( ... ) Serial.print( __VA_ARGS__ ); start_SD_Log(); logfile.print( __VA_ARGS__ ); end_SD_Log()
    #define LOG_PRINT_LN( ... ) Serial.println( __VA_ARGS__ ); start_SD_Log(); logfile.println( __VA_ARGS__ ); end_SD_Log()
  #else
    #define LOG_PRINT( ... ) start_SD_Log(); logfile.print( __VA_ARGS__ ); end_SD_Log()
    #define LOG_PRINT_LN( ... ) start_SD_Log(); logfile.println( __VA_ARGS__ ); end_SD_Log()
  #endif
#else
  #define LOG_PRINT( ... ) Serial.print( __VA_ARGS__ )
  #define LOG_PRINT_LN( ... ) Serial.println( __VA_ARGS__ )
#endif



//________________________Global declarations______________________________
const char string_format_ip[] = ", format \"000.111.222.333\": ";
byte tries = 0;
byte tries_getAVRID = 0;
byte countOfflineDevices = 0;

//char serverURL[] = "lan-user.danit.de";
char serverURL[] = "kolchose.org";
//char serverPath[] = "";
char serverPath[] = "/autarc_lan_user_stats/03/\?";
#define SERVER_ADD_URI "/ping_result/add"
#define SERVER_STATS_URI "/stats/network/"
#define SERVER_GET_ID_URI "/networks/list"
char VersionNR[] = "1.3";  //TODO: Automatically?
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
      LOG_PRINT_LN(F("The complete log output will also be stored on SD"));
    #else
      Serial.println(F("The complete log will be stored on SD, so you can't use the Serial monitor even not for configuration."));
    #endif
  #endif
  
  #ifdef SHOW_MEMORY
    LOG_PRINT(F("Free Arduino Memory in bytes (startup): "));
    LOG_PRINT_LN(freeRam());
  #endif

  //________________________Configuration of the board______________________________
  LOG_PRINT(F("Press any key start configuration"));
  for (char i = 0; i < 1 and Serial.available() <= 0; i++) {
    delay(1000);
    LOG_PRINT(".");
  }

  configuration = Serial.read();
  if (configuration >= 0) {
    LOG_PRINT_LN(F("Starting configuration"));
    startConfiguration();

  }
  else {
    LOG_PRINT_LN(F("no configuration"));
  }

  //_____________________Loading the values for the board__________________________
  if (read_EEPROM(0) != 1) {
    //use default values:
    LOG_PRINT_LN(F("No configuration stored yet. Using default values..."));
    Load_Default_Config();
  }
  else {
    //Read values from EEPROM:
    LOG_PRINT_LN(F("Using configuration from EEPROM."));
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
  LOG_PRINT_LN(F("Try to get IP address from network..."));
  LOG_PRINT(F(" MAC address of shield: "));
  print_mac(mac_shield);
  LOG_PRINT_LN();

  startConnection();
  printConnectionDetails();


  #if ARDUINO > 105
    LOG_PRINT(F("Visit http:\/\/"));
  #else
    LOG_PRINT(F("Visit http://"));
  #endif
  print_ip(ip_shield);
  LOG_PRINT_LN(F("/ with your browser to add a name to your device. Tell all users to do the same."));
  #if ARDUINO > 105
    LOG_PRINT(F("Also check out http:\/\/"));
  #else
    LOG_PRINT(F("Also check out http://"));
  #endif
  LOG_PRINT(serverURL);
  LOG_PRINT(serverPath);
  LOG_PRINT(F(SERVER_STATS_URI)); // GET /stats/network/[network_name][/range]
  LOG_PRINT(AVRID);
  LOG_PRINT_LN(F(" to see your stats online!"));
  
  
  LOG_PRINT_LN(F("Starting server"));
  server.begin();


  readSubnettingIP();

  LOG_PRINT(F("\nStarting loop trough IP range "));
  print_ip(start_ip);
  LOG_PRINT(F(" - "));
  print_ip(end_ip);
  LOG_PRINT_LN("\n");
}

//___________________________Scan the network_________________________________
void loop() {
  char filterResult;

  #ifdef SHOW_MEMORY
    LOG_PRINT(F("Free Arduino Memory in bytes (start loop): "));
    LOG_PRINT_LN(freeRam());
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
            LOG_PRINT(F("Arduino on IP "));
            print_ip(currIP);
            LOG_PRINT_LN();
            send_info_to_server_troublehandler("Arduino");
          }
          else {
            //free, gateway or dnsSrv
            if (pingDevice() == 1) {
              timeDeviceFound = millis();
              if (filterResult == 0) {
                //free IP
                LOG_PRINT(F("Device found on IP "));
                print_ip(currIP);
                LOG_PRINT(F(" MAC: "));
                print_mac(currMAC);
                LOG_PRINT_LN();
                send_info_to_server_troublehandler("");
              }
              else if (filterResult == 2) {
                //IP of gateway
                LOG_PRINT(F("Gateway found on IP "));
                print_ip(currIP);
                LOG_PRINT(F(" MAC: "));
                print_mac(currMAC);
                LOG_PRINT_LN();
                send_info_to_server_troublehandler("Gateway");
              }
              else if (filterResult == 4) {
                //IP of dnsSrv
                LOG_PRINT(F("DNS-Server found on IP "));
                print_ip(currIP);
                LOG_PRINT(F(" MAC: "));
                print_mac(currMAC);
                LOG_PRINT_LN();
                send_info_to_server_troublehandler("DNS-Server");
              }
              else if (filterResult == 6) {
                //IP of gateway & dnsSrv
                LOG_PRINT(F("Gateway found on IP "));
                print_ip(currIP);
                LOG_PRINT(F(" MAC: "));
                print_mac(currMAC);
                LOG_PRINT_LN();
                send_info_to_server_troublehandler("Gateway");
                ServerListenLoop(4);
                LOG_PRINT(F("DNS-Server found on IP "));
                print_ip(currIP);
                LOG_PRINT(F(" MAC: "));
                print_mac(currMAC);
                LOG_PRINT_LN();
                send_info_to_server_troublehandler("DNS-Server");
              }
            }
            else {
              LOG_PRINT("No (pingable) device on IP ");
              print_ip(currIP);
              LOG_PRINT_LN();
              
              offlineIP[countOfflineDevices][0] = currIP[0];
              offlineIP[countOfflineDevices][1] = currIP[1];
              offlineIP[countOfflineDevices][2] = currIP[2];
              offlineIP[countOfflineDevices][3] = currIP[3];
              timeScanned[countOfflineDevices] = millis();
              countOfflineDevices++;
              if (countOfflineDevices == MAX_DEVICES_INFO) {
                //number of max offline devices reached -> send info to server
                LOG_PRINT_LN(F("Max offline devices reached. Sending devices to server."));
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
    LOG_PRINT(F("Free Arduino Memory in bytes (end loop): "));
    LOG_PRINT_LN(freeRam());
  #endif
  LOG_PRINT_LN(F("Restart loop"));
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
    LOG_PRINT(ch);
    if (ch == '\r') {
      LOG_PRINT_LN("\n");
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

  LOG_PRINT_LN(F("Load default configuration (0 = no): "));
  if (GetNumber() == 0) {
    LOG_PRINT_LN(F("Easy configuration? (0 = no): "));
    easyConfig = GetNumber();

    LOG_PRINT_LN(F("MAC Board, format \"00:00:00:00:00:00\": "));
    GetMAC(mac_shield);
    LOG_PRINT_LN();
    print_mac(mac_shield);
    LOG_PRINT_LN("\n");

    if (easyConfig == 0) {
      LOG_PRINT_LN(F("Use DHCP (0 = no): "));
      useDhcp = GetNumber();
      if (useDhcp == 0) {
        LOG_PRINT_LN(F("Don't use DHCP"));
        manualIPConfig();
      }
      else {
        LOG_PRINT_LN(F("Use DHCP"));
        tryDHCP();
      }

      LOG_PRINT_LN(F("Use Subnetting (0 = no): "));
      useSubnetting = GetNumber();
      if (useSubnetting == 0) {
        LOG_PRINT_LN(F("Don't use Subnetting"));
        LOG_PRINT_LN("\n");

        LOG_PRINT(F("Start IP for scan"));
        LOG_PRINT_LN(string_format_ip);
        GetIP(start_ip);
        LOG_PRINT_LN();
        print_ip(start_ip);
        LOG_PRINT_LN("\n");

        LOG_PRINT(F("End IP for scan"));
        LOG_PRINT_LN(string_format_ip);
        GetIP(end_ip);
        LOG_PRINT_LN();
        print_ip(end_ip);
        LOG_PRINT_LN("\n");
      }
      else {
        LOG_PRINT_LN(F("Use Subnetting"));
        LOG_PRINT_LN("\n");
      }

      LOG_PRINT_LN(F("Number of ping-requests: "));
      pingrequest = GetNumber();
      LOG_PRINT(F("Number of ping-requests: "));
      LOG_PRINT(pingrequest);
      LOG_PRINT_LN("\n");

      LOG_PRINT_LN(F("Number of server retries: "));
      retryHost = GetNumber();
      LOG_PRINT(F("Number of server retries: "));
      LOG_PRINT(retryHost);
      LOG_PRINT_LN(retryHost);
      LOG_PRINT_LN("\n");

    }
    else {
      tryDHCP();
      useSubnetting = 1;
      pingrequest = 1;
      retryHost = 2;
    }

    LOG_PRINT_LN(F("Register AVR online? (0 = no): "));
    if (GetNumber() == 0) {
      LOG_PRINT_LN(F("AVR-ID (5 chars) (NEW: Network Name): "));
      GetString(AVRID, sizeof(AVRID));
      LOG_PRINT(AVRID);
      LOG_PRINT_LN("\n");

      LOG_PRINT_LN(F("AVR Password (6 chars): "));
      GetString(AVRpsw, sizeof(AVRpsw));
      LOG_PRINT(AVRpsw);
      LOG_PRINT_LN("\n");
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


    LOG_PRINT_LN("\n");
    LOG_PRINT_LN(F("All values have been stored!"));
    LOG_PRINT_LN(F("Configuration finished"));

  }
  else {
    //Delete settings and set configured = 0 in EEPROM
    write_EEPROM(0, 0);
    LOG_PRINT_LN(F("Default configuration loaded"));
  }
  LOG_PRINT_LN("\n");
}

void manualIPConfig(void) {
  LOG_PRINT(F("IP Board"));
  LOG_PRINT_LN(string_format_ip);
  GetIP(ip_shield);
  LOG_PRINT_LN();
  print_ip(ip_shield);
  LOG_PRINT_LN("\n");

  LOG_PRINT(F("IP Gateway"));
  LOG_PRINT_LN(string_format_ip);
  GetIP(gateway);
  LOG_PRINT_LN();
  print_ip(gateway);
  LOG_PRINT_LN("\n");

  LOG_PRINT(F("Subnetmask"));
  LOG_PRINT_LN(string_format_ip);
  GetIP(subnet);
  LOG_PRINT_LN();
  print_ip(subnet);
  LOG_PRINT_LN("\n");

  LOG_PRINT(F("IP DNS-Server"));
  LOG_PRINT_LN(string_format_ip);
  GetIP(dnsSrv);
  LOG_PRINT_LN();
  print_ip(dnsSrv);
  LOG_PRINT_LN("\n");
}


//_________________________________IP functions______________________________________
void print_ip(byte* ip) {
  LOG_PRINT(ip[0]);
  LOG_PRINT(".");
  LOG_PRINT(ip[1]);
  LOG_PRINT(".");
  LOG_PRINT(ip[2]);
  LOG_PRINT(".");
  LOG_PRINT(ip[3]);
}

void print_mac(byte* mac) {
  LOG_PRINT(mac[0], HEX);
  LOG_PRINT(":");
  LOG_PRINT(mac[1], HEX);
  LOG_PRINT(":");
  LOG_PRINT(mac[2], HEX);
  LOG_PRINT(":");
  LOG_PRINT(mac[3], HEX);
  LOG_PRINT(":");
  LOG_PRINT(mac[4], HEX);
  LOG_PRINT(":");
  LOG_PRINT(mac[5], HEX);
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
  LOG_PRINT_LN(F("Testing DHCP..."));
  if (Ethernet.begin(mac_shield) == 0) {
    LOG_PRINT_LN(F("  DHCP failed, no automatic IP address assigned!"));
    LOG_PRINT_LN(F("You have to configurate the connection settings manual:"));
    useDhcp = 0;
    manualIPConfig();
  }
  else {
    //DHCP possible
    LOG_PRINT_LN(F("DHCP successful"));
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
        LOG_PRINT(F("Free Arduino Memory in bytes (device found): "));
        LOG_PRINT_LN(freeRam());
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
      LOG_PRINT_LN("\n");
      LOG_PRINT_LN(F("--------------"));
      LOG_PRINT_LN(F("Your account data: "));
      LOG_PRINT_LN(AVRID);
      LOG_PRINT_LN(AVRpsw);
      LOG_PRINT_LN(F("--------------"));
      LOG_PRINT_LN(F("disconnecting."));
      client.stop();
    }
  }
  else {
    //Connection to HTTP-Server failed
    LOG_PRINT_LN(F("Can't connect to HTTP-Server. Please try it later or restart the board."));
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
    LOG_PRINT(F("Connected to HTTP Server"));
    LOG_PRINT_LN(serverURL);

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

    LOG_PRINT_LN(client.status());
    //client.stop();
    return 1;

  }
  else {
    LOG_PRINT_LN(F("NOT connected to HTTP Server"));
    LOG_PRINT_LN("\n");
    LOG_PRINT_LN(client.status());
    client.stop();
    if (tries_getAVRID < 2) {
      tries_getAVRID++;
      LOG_PRINT_LN(F("Retry to connect..."));
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
        LOG_PRINT_LN(F("DHCP failed, no automatic IP address assigned!"));
        LOG_PRINT_LN(F("Trying to reconnect in 20 seconds..."));
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
    LOG_PRINT_LN(F("DHCP renewed"));
    readConnectionValues();
    return 1;
  }
  else if (result == 4) {
    LOG_PRINT_LN(F("DHCP rebind"));
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
  LOG_PRINT_LN(F(" Address assigned?"));
  LOG_PRINT(" ");
  LOG_PRINT_LN(Ethernet.localIP());
  LOG_PRINT(" ");
  LOG_PRINT_LN(Ethernet.subnetMask());
  LOG_PRINT(" ");
  LOG_PRINT_LN(Ethernet.gatewayIP());
  LOG_PRINT_LN(F("Setup complete\n"));
  #ifdef SHOW_MEMORY
    LOG_PRINT(F("Free Arduino Memory in bytes (setup complete): "));
    LOG_PRINT_LN(freeRam());
  #endif
}

void send_info_to_server_troublehandler(char *name) { 
  while (send_info_to_server_check_troubles(name) == 0) {}
}

byte send_info_to_server_check_troubles(char *name) {
  if (send_info_to_server(name) == 0) {
    //Connection to HTTP-Server failed -> ping gateway
    LOG_PRINT_LN(F("Connection to HTTP-Server failed"));
    ICMPEchoReply echoReplyGateway = ping(gateway, pingrequest);
    if (echoReplyGateway.status == SUCCESS) {
      // Gateway response -> HTTP-Server offline?
      LOG_PRINT_LN(F("HTTP-Server may be broken. Trying again in 30 seconds."));
      ServerListenLoop(1); //30seconds
      return 0;
    }
    else {
      // Gateway also not available -> Connection problem -> Try to reconnect
      LOG_PRINT_LN(F("There is a connection error. Trying to start new connection..."));
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
    LOG_PRINT(F("Free Arduino Memory in bytes (send info): "));
    LOG_PRINT_LN(freeRam());
  #endif
  if (client.connect(serverURL, 80) == 1) {
    tries = 0;
    unsigned long timeDifference;
    
    LOG_PRINT(F("Connected to HTTP Server "));
    LOG_PRINT(serverURL);
    LOG_PRINT(serverPath);
    LOG_PRINT_LN(SERVER_ADD_URI);
    
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
    client.print(AVRID);
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
      
      for (int macreset = 0; macreset < 6; macreset++) {
        currMAC[macreset] = 0x00;
      }
      
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
    

    LOG_PRINT(F("Ethernet Client status: "));
    LOG_PRINT_LN(client.status()); // 23 Code together with HTTP Timeout: could mean no/wrong MAC for Shield is set!
    char tmpc;
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.connected())
    {
      if (client.available()) {
        tmpc = client.read();
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
    LOG_PRINT_LN(F("NOT connected to HTTP Server"));
    LOG_PRINT_LN("\n");
    LOG_PRINT_LN(client.status());
    client.stop();
    if (tries < retryHost) {
      tries++;
      LOG_PRINT_LN(F("Retry to connect..."));
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
    LOG_PRINT_LN(F("new client"));
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
          serverClient.print(F(SERVER_STATS_URI));
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
    LOG_PRINT_LN(F("client disconnected"));
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
  void init_SD(void) {
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
    
    pinMode(10, OUTPUT);
    pinMode(4, OUTPUT);
    
    //activate SD
    digitalWrite(10, HIGH);  //turn off ETH
    digitalWrite(4, LOW);
    
      if (!SD.begin(4)) {
        Serial.println(F("initializing of SD card failed"));
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
            set_next_log_filename();
          }
          else {
            break;
          }
        }
        //delete the new file, if it exist (only case if all 99 files exist; so delete 00)
        SD.remove(SDfileName);
        Serial.print("New logfile will be created: ");
        Serial.println(SDfileName);
      }
      else {
        SD.mkdir("log");
      }
      
      logfile = SD.open(SDfileName, FILE_WRITE);
      
      if (logfile) {
        logfile.println(F("### Restart of board ###"));
        logfile.close();
      }
      else {
        Serial.println(F("Can't write to SD card! No log will be stored!"));
      }
    
    //activate ETH
    digitalWrite(4, HIGH);  //turn off SD
    digitalWrite(10, LOW);
  }

  void start_SD_Log(void) {
    //activate SD
    digitalWrite(10, HIGH);  //turn off ETH
    digitalWrite(4, LOW);

    /* Todo: maybe reinitialize
    if (!SD.begin(4)) {
      Serial.println(F("initializing of SD card failed"));
    }
    else {
      Serial.println(F("SD card initialized"));
    }
    */
    
    logfile = SD.open(SDfileName, FILE_WRITE);
    
    if (logfile) {
      if (logfile.size() >= MAX_LOG_SIZE) {
        //next time new file -> set filename
        set_next_log_filename();
        
        //delete the new file, if it exist
        SD.remove(SDfileName);
        Serial.print("New logfile will be created: ");
        Serial.println(SDfileName);
      }
    }
    else {
      Serial.println(F("Can't write to SD card! No log will be stored!"));
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

  void set_next_log_filename(void) {
    int fileNr = (SDfileName[5] - '0');
    if (fileNr == 9) {
      fileNr = (SDfileName[4] - '0');
      if (fileNr == 9) {
        //99 files reached, start with 00
        SDfileName[4] = '0';
      }
      else {
        fileNr = fileNr + 49;
        SDfileName[4] = char(fileNr);
      }
      SDfileName[5] = '0';
    }
    else {
      fileNr = fileNr + 49;
      SDfileName[5] = char(fileNr);
    }
  }
#endif

