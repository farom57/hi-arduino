#ifndef HI_ARDUINO_H
#define HI_ARDUINO_H

#include "Arduino.h"
#define MAX_LENGTH 64
#define DEBUG_ALL
#define DEBUG_ERROR




// Read parameter from HVAC
int hiRead(int address, char* data);
int hiRead(int address, int* result);

// Set parameter
int hiSet(int address, char* data, int length);
int hiSet(int address, int value, int length);

// receive data from HVAC
int hiReceive(char* data);
int hiReceive(int* result);

// convert single hexadecimal char to byte
byte hex2byte(char in);

// convert two hexadecimal char to byte
byte hex2byte(char MSB, char LSB);

// convert an array of len hexadecimal char to an array of len/2 bytes 
void hex2byte(char* in, byte* out, int len);

// convert an array of len hexadecimal char into an int
int hex2int(char* in, int len);

// compute the checksum of a byte array
int checksum(byte* in, int len);

#endif
