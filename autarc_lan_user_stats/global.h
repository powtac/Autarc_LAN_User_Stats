#ifndef GLOBAL
  #define GLOBAL
  
  #include <arduino.h>
  #include <stdarg.h>
  #include <util.h>
  #include <SPI.h>
  #include <Ethernet.h>
  #include <EEPROM.h>
  // See https://github.com/BlakeFoster/Arduino-Ping/
  #include "ICMPPing.h"
  #include "default_config.cpp"
  #include "ConfigHelper.h"
  #include "IPHelper.h"
  #include "memcheck.h"
  //#include "TimerOne.h"
  
  char serverURL[] = "lan-user.danit.de";
  char VersionNR[] = "/1.0";
  
  byte currIP[4];
  byte currMAC[6];


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
  
  char AVRID[6];
  char AVRpsw[7];
  
  
  // Ping library configuration
  SOCKET pingSocket              = 0;
  
  // Global namespace to use it in setup() and loop()
  ICMPPing ping(pingSocket, (uint16_t)random(0, 255));

  EthernetServer server(80);

#endif
