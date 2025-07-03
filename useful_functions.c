//This file is not necessary for Autarc_LAN_User_Stat
//It's only a file to collect useful functions which have been used in this project
//Maybe those functions are helpful for other projects

void ip_to_string(byte ip[4], char *out) {
  sprintf(out, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
}


void mac_to_string(byte mac[6], char *out) {
  sprintf(out, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}


char* mac_to_char(byte mac[6]) {
  //Convert mac address byte to chararray
  //Same as sprintf(return_mac, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  int n = 0;
  int m = 0;
  const char hexchars[] = "0123456789ABCDEF"; // Hexcharset

  for (n=0; n<=5; n++) {
    return_mac_to_char[m] = hexchars[mac[n] >> 4 & 0xF];
    return_mac_to_char[m + 1] = hexchars[mac[n] & 0xF];
    if (m < 15) {
      return_mac_to_char[m + 2] = ':';
      m = m + 3;
    }
  }
  return_mac_to_char[17] = '\0';
  return return_mac_to_char;
}


char* ip_to_char(byte ip[4]) {
  //Convert ip address byte to chararray
  int n = 0;
  int m = 0;
  char buf[4];

  for (n=0; n<=3; n++) {
    itoa(ip[n], buf, 10);
    if (ip[n] > 99) {
      return_ip_to_char[m] = buf[0];
      return_ip_to_char[m + 1] = buf[1];
      return_ip_to_char[m + 2] = buf[2];
      m = m + 4;
    }
    else if (ip[n] > 9) {
      return_ip_to_char[m] = buf[0];
      return_ip_to_char[m + 1] = buf[1];
      m = m + 3;
    }
    else {
      return_ip_to_char[m] = buf[0];
      m = m + 2;
    }
    return_ip_to_char[m - 1] = '.';
  }
  return_ip_to_char[m - 1] = '\0';
  return return_ip_to_char;
}


char* ip_to_char(byte ip[4]) {
  //Convert ip address byte to chararray without itoa function
  unsigned char n;
  char m;
  unsigned char p = 0;
  unsigned char add;
  unsigned char val;
  const char numchars[] = "0123456789ABCDEF"; // Numcharset

  for (n=0; n<=3; n++) {
    val = ip[n];
    if (val > 99) {
      add = 2;
    }
    else if (val > 9) {
      add = 1;
    }
    else {
      add = 0;
    }
    
    for (m = add;m>=0;m--) {
      return_ip_to_char[p + m] = numchars[val % 10];
      val /= 10;
    }
    p = p + 2 + add;

    return_ip_to_char[p - 1] = '.';
  }
  return_ip_to_char[p - 1] = '\0';
  return return_ip_to_char;
}

