// Struktur bildet eine IP-Adresse, wird für den Array der Known IPs gebraucht
typedef struct {
  byte ipadress;
  //String mac;
} IPAdress;

/*IPAdress convertByteToIp(byte iparray[4]) {
   IPAdress IP;
   IP.ipadress[0] = iparray[0];
   IP.ipadress[1] = iparray[1];
   IP.ipadress[2] = iparray[2];
   IP.ipadress[3] = iparray[3];
   //IP.mac = "";
   return IP;
}*/

// Struktur ist ein Dynamisches Array mit IPAdressen
typedef struct {
  IPAdress *array;
  size_t used;
  size_t size;
} IPAdresses;

void initArray(IPAdresses *a, size_t initialSize) {
  a->array = (IPAdress *)malloc(initialSize * sizeof(IPAdress));
  a->used = 0;
  a->size = initialSize;
}

void insertArray(IPAdresses *a, IPAdress element) {
  if (a->used == a->size) {
    a->size ++;
    a->array = (IPAdress *)realloc(a->array, a->size * sizeof(IPAdress));
  }
  a->array[a->used++] = element;
}

void freeArray(IPAdresses *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

void remove_element(IPAdresses *a, int indexToRemove) {
    if (a->used == 1 && indexToRemove == 0) {
      a->array = NULL;
      a->used = a->size = 0;
    } else {
      IPAdresses temp; // allocate an array with a size 1 less than the current one
      initArray(&temp, a->used);
      for (int i = 0; i < a->used; ++i) {
        if (i != indexToRemove)
          insertArray(&temp, a->array[i]);
      }
      freeArray(a);
      memcpy(a, &temp, sizeof(temp));
    }
}

/*
int indexOf(IPAdresses *a, IPAdress element) {
  for (int i = 0; i < a->used; ++i) {
    if (memcmp(element.ipadress, a->array[i].ipadress, sizeof(element.ipadress)) == 0) 
      return i; // found on position i
  }
  return -1; // not found
}
*/

