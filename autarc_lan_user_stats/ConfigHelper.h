#ifndef CONFIGHELPER
  #define CONFIGHELPER
  #include "global.h"
  #include "IPHelper.h"
  #include "default_config.cpp"
  #include "ConfigHelper.cpp"  
  
  extern const char string_format_ip[];
  
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



#endif
