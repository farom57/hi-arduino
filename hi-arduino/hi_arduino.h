#ifndef HI_ARDUINO_H
#define HI_ARDUINO_H

#include "Arduino.h"
#define MAX_LENGTH 64


// receive data from HVAC
// data: null terminated char array to store the result
// return the number of char readed, 0 if nothing after the timeout, -1 in case of checksum error
int hiReceive(char* data);

// receive integer from HVAC
// result: value received
// return the number of char readed, 0 if nothing after the timeout, -1 in case of checksum error
int hiReceive(int* result);

// Read parameter from HVAC
int hiRead(int address, char* data);
int hiRead(int address, int* result);

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
