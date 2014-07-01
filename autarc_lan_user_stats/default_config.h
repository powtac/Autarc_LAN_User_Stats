byte mac_shield[6];
byte ip_shield[4];
byte gateway[4];
byte subnet[4];
byte useDhcp;
byte pingrequest;
byte useSubnetting;
byte start_ip[4];
byte end_ip[4];

char AVRID[6];

void Load_Default_Config(void);


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
  pingrequest = 2;
  useSubnetting = 1;
  start_ip[0] = 192;
  start_ip[1] = 168;
  start_ip[2] = 178;
  start_ip[3] = 2;
  end_ip[0] = 192;
  end_ip[1] = 168;
  end_ip[2] = 178;
  end_ip[3] = 254;
  AVRID[0] = 'T';
  AVRID[1] = 'i';
  AVRID[2] = 'm';
  AVRID[3] = '0';
  AVRID[4] = '1';
  AVRID[5] = '\0';
}

// Simon
//static char AVRID[6]           = "Simon";
//static uint8_t mac_shield[6]   = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F }; // 90:a2:da:00:46:8f
//byte ip_shield[4]              = { 10, 0, 1, 13 };
//byte gateway[4]                = { 10, 0, 1, 1 };
//byte subnet[4]                 = { 255, 255, 0, 0 };

//// Jonas
//static char AVRID[6]           = "Jonas";
//static uint8_t mac_shield[6]   = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F };
//byte ip_shield[4]              = { 192, 168, 1, 30 };
//byte gateway[4]                = { 192, 168, 1, 1 };
//byte subnet[4]                 = { 255, 255, 0, 0 };
