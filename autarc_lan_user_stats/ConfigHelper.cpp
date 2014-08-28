#include "ConfigHelper.h"

const char string_format_ip[] = ", format \"000.111.222.333\": ";



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
  for (i=0; ; ++i) {
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
  IP[0] = atoi(strtok_r(input,".",&i));
  IP[1] = atoi(strtok_r(NULL,".",&i));
  IP[2] = atoi(strtok_r(NULL,".",&i));
  IP[3] = atoi(strtok_r(NULL,".",&i));
}

void GetMAC(byte *MAC) {
  char input[18];
  GetString(input, sizeof(input));

  char *i;
  MAC[0] = strtol(strtok_r(input,":",&i), NULL, 16);
  MAC[1] = strtol(strtok_r(NULL,":",&i), NULL, 16);
  MAC[2] = strtol(strtok_r(NULL,":",&i), NULL, 16);
  MAC[3] = strtol(strtok_r(NULL,":",&i), NULL, 16);
  MAC[4] = strtol(strtok_r(NULL,":",&i), NULL, 16);
  MAC[5] = strtol(strtok_r(NULL,":",&i), NULL, 16);
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
        
        //TODO: ADD:
        //manualIPConfig();

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
