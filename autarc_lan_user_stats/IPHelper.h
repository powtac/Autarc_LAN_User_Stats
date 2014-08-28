#ifndef IPHELPER
  #define IPHELPER
  #include "memcheck.h"
  #include "default_config.cpp"
  #include "global.h"
  
  #include "ConfigHelper.h"
  
  
  extern char tries;
  extern char tries_getAVRID;
  
  void print_ip(byte* ip);
  void print_mac(byte* mac);
  char tryDHCP(void);
  void getAVRID(void);
  char connect_getAVRID(EthernetClient &client);
  void startConnection(void);
  void printConnectionDetails(void);
  void send_info_to_server_troublehandler(void);
  char send_info_to_server(byte* IP, byte* MAC);
  void ServerListenLoop(int count);
  void ServerListen(void);
  
  #include "IPHelper.cpp"
  
  //char* AVRID, char* AVRpsw, byte retryHost, char* serverURL, char* VersionNR

#endif
