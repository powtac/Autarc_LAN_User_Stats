#ifndef DEFAULT_CONFIG
  #define DEFAULT_CONFIG
  
  //Prints the status of RAM
  //#define SHOW_MEMORY  //Also comment the '#include' Tag in the main program because of arduino bug
  //#define LOG_TO_SD  //Also comment the '#include' Tag in the main program because of arduino bug; This will disable the Serial log
  //#define LOG_TO_SD_AND_SERIAL  //This will need a lot of flash memory (Sketch size)! Attention! This could cause a to high Sketch size!
  #define MAX_LOG_SIZE 500000
  //#define STORE_LOG_NR_EEPROM //If set, the actual log file number will be stored in EEPROM. In case of restart the board will continue with the next log file. Remember: Too many EEPROM writes cause damage!
  #define SEND_HARDWARE_TO_SERVER //Will send Hardware devices like Gateway, DNS Server and Arduino to the server; If comment, NO stats about the IP will be send

  //#define WITH_ESCAPE_SEQUENCE //Neccessary for some Arduino versions newer 105 older 167 (?). [ARDUINO > 105]; Causes a lot of warnings with "unknown escape sequence"
  //#define INCREASE_LOG_SPEED //Need more memory, but increases the speed of the log (especially SD log)  //TODO: Not implemented complete (connects all mesages in one string)
  #define PRINT_SERVER_ANSWER //Prints the answer of the stat server after each send-devices-request
  
  void Load_Default_Config(void);
  byte mac_shield[6];
  byte ip_shield[4];
  byte gateway[4];
  byte subnet[4];
  byte useDhcp;
  byte pingrequest;
  byte useSubnetting;
  byte start_ip[4];
  byte end_ip[4];
  byte dnsSrv[4];
  byte retryHost;
  
  char NetworkName[6];
  char NetworkPwd[7];
  
  
  void Load_Default_Config(void) {
    mac_shield[0] = 0x90;
    mac_shield[1] = 0xA2;
    mac_shield[2] = 0xDA;
    mac_shield[3] = 0x00;
    mac_shield[4] = 0x46;
    mac_shield[5] = 0x9F;
    ip_shield[0] = 192;
    ip_shield[1] = 168;
    ip_shield[2] = 178;
    ip_shield[3] = 98;
    gateway[0] = 192;
    gateway[1] = 168;
    gateway[2] = 178;
    gateway[3] = 1;
    subnet[0] = 255;
    subnet[1] = 255;
    subnet[2] = 255;
    subnet[3] = 0;
    useDhcp = 1;
    pingrequest = 4;
    useSubnetting = 1;
    start_ip[0] = 192;
    start_ip[1] = 168;
    start_ip[2] = 178;
    start_ip[3] = 2;
    end_ip[0] = 192;
    end_ip[1] = 168;
    end_ip[2] = 178;
    end_ip[3] = 254;
    NetworkName[0] = 'T';
    NetworkName[1] = 'i';
    NetworkName[2] = 'm';
    NetworkName[3] = '0';
    NetworkName[4] = '1';
    NetworkName[5] = '\0';
    dnsSrv[0] = 192;
    dnsSrv[1] = 168;
    dnsSrv[2] = 178;
    dnsSrv[3] = 1;
    retryHost = 2;
    NetworkPwd[0] = '1';
    NetworkPwd[1] = '2';
    NetworkPwd[2] = '3';
    NetworkPwd[3] = '4';
    NetworkPwd[4] = '5';
    NetworkPwd[5] = '6';
    NetworkPwd[6] = '\0';
  }
  
  // Simon
  //static char NetworkName[6]           = "Simon";
  //static uint8_t mac_shield[6]   = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F }; // 90:a2:da:00:46:8f
  //byte ip_shield[4]              = { 10, 0, 1, 13 };
  //byte gateway[4]                = { 10, 0, 1, 1 };
  //byte subnet[4]                 = { 255, 255, 0, 0 };
  
  //// Jonas
  //static char NetworkName[6]           = "Jonas";
  //static uint8_t mac_shield[6]   = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F };
  //byte ip_shield[4]              = { 192, 168, 1, 30 };
  //byte gateway[4]                = { 192, 168, 1, 1 };
  //byte subnet[4]                 = { 255, 255, 0, 0 };

#endif

