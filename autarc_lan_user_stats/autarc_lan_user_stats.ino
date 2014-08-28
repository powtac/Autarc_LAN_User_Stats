#include "global.h"

// init_mem();  //hier???
// Serial.println(get_mem_unused());

// WireShark Filter: eth.addr[0:3]==90:A2:DA


byte readSubnet[4];
byte readIP[4];


void setup() {
  int configuration;
  delay(1000);
  Serial.begin(115200);
  Serial.print(F("Memory: "));
  Serial.println(get_mem_unused());

  //________________________Configuration of the board______________________________

  Serial.print(F("Press any key start configuration"));
  for (char i = 0; i < 5 and Serial.available() <= 0; i++) {
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
  Serial.print(F("Speicher (End ServerSend): "));
  Serial.println(get_mem_unused());
  Serial.println(F("Restart loop"));
}




