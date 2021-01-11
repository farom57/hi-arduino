#include "hi_arduino.h"
#include <stdio.h>
#include <stdarg.h>


void printf(char *fmt, ... ) {
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(buf, 128, fmt, args);
  va_end (args);
  Serial.print(buf);
}

/*void snprintf(char *buf, int len, char *fmt, ... ) {
  va_list args;
  va_start (args, fmt );
  vsnprintf(buf, len, fmt, args);
  va_end (args);
  }*/


// receive data from HVAC
// data: null terminated char table to store the result
// return the number of char readed, 0 if nothing after the timeout, -1 in case of checksum error
int hiReceive(char* data) {
  char buf[MAX_LENGTH];   // full message
  char p_buf[MAX_LENGTH]; // parameter only
  char chk_buf[5];        // checksum only
  int len;                // message length
  int p_len;              // parameter length
  int i = 0;

  char terminator = 0x0D;

  len = Serial1.readBytesUntil(terminator, buf, 64);


  char* buf2;             // message with correct start
  if (buf[0] == 0) { // discard null leading character
    buf2 = buf + 1;
    len = len - 1;
  } else {
    buf2 = buf;
  }

  if (len < 1) {
    Serial.println("Timeout");
    return 0;
  }

  if (buf2[0] != 'O' || buf2[1] != 'K') {
    Serial.print("HVAC response error: ");
    Serial.println(buf2);
    return -3;
  }


  // parameter
  while (isHexadecimalDigit(buf2[i + 5])) {
    p_buf[i] = buf2[i + 5];
    i++;
  }
  p_len = i;

  // checksum
  chk_buf[0] = buf2[p_len + 8 + 0];
  chk_buf[1] = buf2[p_len + 8 + 1];
  chk_buf[2] = buf2[p_len + 8 + 2];
  chk_buf[3] = buf2[p_len + 8 + 3];
  chk_buf[4] = 0;

  // verify checksum
  byte p_bytes[MAX_LENGTH / 2];
  int p;
  int chk_read, chk_verif;
  p = hex2int(p_buf, p_len);
  hex2byte(p_buf, p_bytes, p_len);
  chk_read = hex2int(chk_buf, 4);
  chk_verif = checksum(p_bytes, p_len / 2);

  if (chk_read == chk_verif) {
    for (int i = 0; i < p_len; i++) {
      data[i] = p_buf[i];
    }
    data[p_len] = 0;
    Serial.print("Value received OK, p_len=");
    Serial.print(p_len);
    Serial.print(": ");
    Serial.print(data);
    Serial.println();
    return p_len;
  } else {
    Serial.print("Checksum error, calculated chksum=");
    Serial.print(chk_verif, HEX);
    Serial.print(": ");
    Serial.write(buf2, len);
    Serial.println();
    return -1;
  }
}

int hiReceiveST(char* data) {
  char buf[MAX_LENGTH];   // full message
  char p_buf[MAX_LENGTH]; // parameter only
  char chk_buf[5];        // checksum only
  int len;                // message length
  int p_len;              // parameter length
  int i = 0;

  char terminator = 0x0D;

  len = Serial1.readBytesUntil(terminator, buf, 64);


  char* buf2;             // message with correct start
  if (buf[0] == 0) { // discard null leading character
    buf2 = buf + 1;
    len = len - 1;
  } else {
    buf2 = buf;
  }

  if (len < 1) {
    //Serial.println("Timeout");
    return 0;
  }

  if (buf2[0] == 'M') {
    return -3;
  }

  Serial.write(buf2, len);
  Serial.println();
  return 1;

}

// receive integer from HVAC
// result: value received
// return the number of char readed, 0 if nothing after the timeout, -1 in case of checksum error
int hiReceive(int* result) {
  char data[MAX_LENGTH];
  int len = hiReceive(data);

  if (len < 1) { // error
    return len;
  }

  *result = hex2int(data, len);
  return len;
}

// Read parameter from HVAC
int hiRead(int address, char* data) {
  if (address > 0xFFFF || address < 0x0000) {
    printf("Address out of range: %X\n");
    return -2;
  }
  byte address_bytes[2];
  address_bytes[0] = (address >> 8) & 0xFF;
  address_bytes[1] = address & 0xFF;
  int chksum = checksum(address_bytes, 2);
  char msg[MAX_LENGTH];
  snprintf(msg, MAX_LENGTH, "MT P=%04X C=%04X\r", address, chksum);
  Serial.print("Sending: ");
  Serial.println(msg);

  Serial1.print(msg);

  return hiReceive(data);
}

// Read parameter from HVAC
int hiRead(int address, int* result) {
  if (address > 0xFFFF || address < 0x0000) {
    printf("Address out of range: %X\n");
    return -2;
  }
  byte address_bytes[2];
  address_bytes[0] = (address >> 8) & 0xFF;
  address_bytes[1] = address & 0xFF;
  int chksum = checksum(address_bytes, 2);
  char msg[MAX_LENGTH];
  snprintf(msg, MAX_LENGTH, "MT P=%04X C=%04X\r", address, chksum);
  Serial.print("Sending: ");
  Serial.println(msg);

  Serial1.print(msg);

  return hiReceive(result);
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
