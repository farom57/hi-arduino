#include "hi_arduino.h"
#include <stdio.h>
#include <stdarg.h>

// Classic printf function output to Serial
void printf(char *fmt, ... ) {
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(buf, 128, fmt, args);
  va_end (args);
  Serial.print(buf);
}

// receive data from HVAC as character array
// data: char array that will to store the result. Size should be MAX_LENGTH
// return the number of char read or a negative number representing the error code
int hiReceive(char* data) {
  char buf[MAX_LENGTH];   // full message
  char p_buf[MAX_LENGTH]; // parameter only
  char chk_buf[5];        // checksum only
  int len;                // message length
  int p_len;              // parameter length
  int i = 0;

  char terminator = 0x0D; // ="\r"

  // Correct response message format "OK P=[value] C=[checksum]\r"
  len = Serial1.readBytesUntil(terminator, buf, MAX_LENGTH);

  // sometimes the input message starts with an extra null character, discard this character
  char* buf2;             // message with correct start
  if (buf[0] == 0) {
    buf2 = buf + 1;
    len = len - 1;
  } else {
    buf2 = buf;
  }

  if (len < 1) {
#ifdef DEBUG_ERROR
    Serial.println("Error 1, Timeout");
#endif
    return -1;
  }

  // check if message starts with "OK"
  if (buf2[0] != 'O' || buf2[1] != 'K') {
#ifdef DEBUG_ERROR
    Serial.println("Error 2, unexpected HVAC response. Expected: OK P=... ");
    Serial.print("Received: ");
    Serial.write(buf2, len);
    Serial.println();
    Serial.println("Try increasing the delay between commands and check the address validity");
#endif
    return -2;
  }


  // copy parameter
  while (isHexadecimalDigit(buf2[i + 5])) {
    p_buf[i] = buf2[i + 5];
    i++;
  }
  p_len = i;

  // copy checksum
  chk_buf[0] = buf2[p_len + 8 + 0];
  chk_buf[1] = buf2[p_len + 8 + 1];
  chk_buf[2] = buf2[p_len + 8 + 2];
  chk_buf[3] = buf2[p_len + 8 + 3];
  chk_buf[4] = 0;

  // verify checksum
  byte p_bytes[MAX_LENGTH / 2];
  int chk_read, chk_verif;
  hex2byte(p_buf, p_bytes, p_len);
  chk_read = hex2int(chk_buf, 4);
  chk_verif = checksum(p_bytes, p_len / 2); // the number of bytes is half the number of hexadecimal characters

  if (chk_read == chk_verif) {
    // copying result
    for (int i = 0; i < p_len; i++) {
      data[i] = p_buf[i];
    }
    data[p_len] = 0;

#ifdef DEBUG_ALL
    Serial.print("Value received OK, parameter length = ");
    Serial.print(p_len);
    Serial.print(" value = ");
    Serial.println(data);
    Serial.print("Full message: ");
    Serial.write(buf2, len);
    Serial.println();
#endif
    return p_len;
  } else {
#ifdef DEBUG_ERROR
    Serial.print("Error 3, Checksum incorrect. Expected: ");
    Serial.println(chk_verif, HEX);
    Serial.print("Received");
    Serial.write(buf2, len);
    Serial.println();
#endif
    return -3;
  }
}


// receive data from HVAC as integer
// result: the value read
// return the number of char read or a negative number representing the error code
int hiReceive(int* result) {
  char data[MAX_LENGTH];
  int len = hiReceive(data);

  if (len < 1) { // error
    return len;
  }

  *result = hex2int(data, len);
  return len;
}

// Read parameter from HVAC as a char array
// address: address of the parameter to read, See hi_arduino.h for a list of valid address
// data: char array that will to store the result. Size should be MAX_LENGTH
int hiRead(int address, char* data) {
  if (address > 0xFFFF || address < 0x0000) {
#ifdef DEBUG_ERROR
    printf("Error 4, Address out of range: %X\n");
#endif
    return -4;
  }

  // Command message form "MT P=[address] C=[checksum]\r"
  byte address_bytes[2];
  address_bytes[0] = (address >> 8) & 0xFF;
  address_bytes[1] = address & 0xFF;
  int chksum = checksum(address_bytes, 2);
  char msg[MAX_LENGTH];
  snprintf(msg, MAX_LENGTH, "MT P = %04X C = %04X\r", address, chksum);

#ifdef DEBUG_ALL
  Serial.print("Sending: ");
  Serial.println(msg);
#endif

  // sending the command
  Serial1.print(msg);

  return hiReceive(data);
}

// Read parameter from HVAC as an integer
// address: address of the parameter to read, See hi_arduino.h for a list of valid address
// result: the value read
int hiRead(int address, int* result) {
  if (address > 0xFFFF || address < 0x0000) {
#ifdef DEBUG_ERROR
    printf("Error 4, Address out of range: %X\n");
#endif
    return -4;
  }

  // Command message form "MT P=[address] C=[checksum]\r"
  byte address_bytes[2];
  address_bytes[0] = (address >> 8) & 0xFF;
  address_bytes[1] = address & 0xFF;
  int chksum = checksum(address_bytes, 2);
  char msg[MAX_LENGTH];
  snprintf(msg, MAX_LENGTH, "MT P = %04X C = %04X\r", address, chksum);

#ifdef DEBUG_ALL
  Serial.print("Sending: ");
  Serial.println(msg);
#endif

  // sending the command
  Serial1.print(msg);

  return hiReceive(result);
}



// Set parameter as a char array
// address: address of the parameter to written, See hi_arduino.h for a list of valid address
// data: char array to be written, it should be composed of capital hexadecimal characters
// length: number of hexadecimal characters in the data
int hiSet(int address, char* data, int length) {
  if (address > 0xFFFF || address < 0x0000) {
#ifdef DEBUG_ERROR
    printf("Error 4, Address out of range: %X\n");
#endif
    return -4;
  }

  byte msg_bytes[MAX_LENGTH / 2];
  msg_bytes[0] = (address >> 8) & 0xFF;
  msg_bytes[1] = address & 0xFF;
  hex2byte(data, msg_bytes + 2, length);

  int chksum = checksum(msg_bytes, length / 2 + 2);
  char msg[MAX_LENGTH];

  // Command message form "ST P=[address],[data] C=[checksum]\r"
  snprintf(msg, MAX_LENGTH, "ST P = % 04X, ", address);
  for (int i = 0; i * 2 < length; i++) {
    snprintf(msg + 10 + i * 2, MAX_LENGTH - 10 - i * 2, " % 02X", msg_bytes[i + 2]);
  }
  snprintf(msg + 10 + length, MAX_LENGTH - 10 - length, " C = % 04X\r", chksum);

#ifdef DEBUG_ALL
  Serial.print("Sending: ");
  Serial.println(msg);
#endif
  Serial1.print(msg);

  // checking result:
  // HVAC should answer "H~"
  char res[MAX_LENGTH];
  char terminator = 0x0D;
  int len;
  len = Serial1.readBytesUntil(terminator, res, MAX_LENGTH);
  if (len >= 2 && res[0] == 'H' && res[1] == '~') {
    return 1;
  } else {
    #ifdef DEBUG_ERROR
    Serial.print("Error 5, unexpected HVAC answers: ");
    Serial.println(res);
    #endif
    return -5;
  }
}

// Set parameter as a integer
// address: address of the parameter to written, See hi_arduino.h for a list of valid address
// value: value to be written
// length: number of hexadecimal characters in the parameter
int hiSet(int address, int value, int length) {
  if (address > 0xFFFF || address < 0x0000) {
#ifdef DEBUG_ERROR
    printf("Error 4, Address out of range: %X\n");
#endif
    return -4;
  }
  byte msg_bytes[MAX_LENGTH / 2];
  msg_bytes[0] = (address >> 8) & 0xFF;
  msg_bytes[1] = address & 0xFF;
  for (int i = 0; i * 2 < length; i++) {
    msg_bytes[2 + i] = (value >> (8 * (length / 2 - 1 - i))) & 0xFF;
  }

  int chksum = checksum(msg_bytes, length / 2 + 2);
  char msg[MAX_LENGTH];

  // Command message form "ST P=[address],[value] C=[checksum]\r"
  snprintf(msg, MAX_LENGTH, "ST P = % 04X, ", address);
  for (int i = 0; i * 2 < length; i++) {
    snprintf(msg + 10 + i * 2, MAX_LENGTH - 10 - i * 2, " % 02X", msg_bytes[i + 2]);
  }
  snprintf(msg + 10 + length, MAX_LENGTH - 10 - length, " C = % 04X\r", chksum);

#ifdef DEBUG_ALL
  Serial.print("Sending: ");
  Serial.println(msg);
#endif
  Serial1.print(msg);

  // checking result:
  // HVAC should answer "H~"
  char res[MAX_LENGTH];
  char terminator = 0x0D;
  int len;
  len = Serial1.readBytesUntil(terminator, res, MAX_LENGTH);
  if (len >= 2 && (res[0] == 'H' && res[1] == '~')) {
    return 1;
  } else {
    #ifdef DEBUG_ERROR
    Serial.print("Error 5, unexpected HVAC answers: ");
    Serial.println(res);
    #endif
    return -5;
  }
}

// convert single hexadecimal char to byte
byte hex2byte(char in) {
  if (isDigit(in)) {
    return in - '0';
  } else {
    return in - 'A' + 10;
  }
}

// convert two hexadecimal char to byte
byte hex2byte(char MSB, char LSB) {
  return (hex2byte(MSB) << 4) + hex2byte(LSB);
}

// convert an array of len hexadecimal char to an array of len/2 bytes
void hex2byte(char* in, byte* out, int len) {
  for (int i = 0; i * 2 < len; i++) {
    out[i] = hex2byte(in[2 * i], in[2 * i + 1]);
  }
}

// convert an array of len hexadecimal char into an int
int hex2int(char* in, int len) {
  int res = 0;

  for (int i = 0; i * 2 + 1 < len; i++) {
    res = (res << 8);
    res += hex2byte(in[2 * i], in[2 * i + 1]);
  }
  return res;
}

// compute the checksum of a byte array
int checksum(byte* in, int len) {
  int sum = 0xFFFF;
  for (int i = 0; i < len; i++) {
    sum -= in[i];
  }
  return sum;
}
