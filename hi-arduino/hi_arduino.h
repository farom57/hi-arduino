#ifndef HI_ARDUINO_H
#define HI_ARDUINO_H

#include "Arduino.h"
#define MAX_LENGTH 64
#define HI_DELAY 200 // duration in ms between message
//#define DEBUG_ALL
#define DEBUG_ERROR

#define ADDR_POWER 0x0000
#define LEN_POWER 2
#define POWER_ON 0x01
#define POWER_OFF 0x00

#define ADDR_MODE 0x0001
#define LEN_MODE 4
#define MODE_HOT 0x0010
#define MODE_DRY 0x0020
#define MODE_COOL 0x0040
#define MODE_FAN 0x0050
#define MODE_AUTO 0x8000

#define ADDR_SPEED 0x0002
#define LEN_SPEED 2
#define SPEED_AUTO 0x00
#define SPEED_HIGH 0x01
#define SPEED_MEDIUM 0x02
#define SPEED_LOW 0x03
#define SPEED_SILENT 0x04

#define ADDR_TARGET 0x0003
#define LEN_TARGET 4
#define TARGET_MIN 16
#define TARGET_MAX 32

#define ADDR_PERMISSION 0x0006
#define LEN_PERMISSION 2
#define PERMISSION_ALLOWED 0x00
#define PERMISSION_PROHIBITED 0x01

#define ADDR_INDOOR 0x0100
#define LEN_INDOOR 4

#define ADDR_OUTDOOR 0x0102
#define LEN_OUTDOOR 4

#define ADDR_ACTIVE 0x0301
#define LEN_ACTIVE 4
#define ACTIVE_OFF 0x0000
#define ACTIVE_ON 0xFFFF

#define ADDR_SERIAL 0x0900
#define LEN_SERIAL 20

#define ADDR_BEEP 0x0800
#define LEN_BEEP 2
#define BEEP_ON 0x07
#define BEEP_OFF 0x00

#define ADDR_ABSENCE 0x0304
#define LEN_ABSENCE 8
#define ABSENCE_OFF 0x00000000
#define ABSENCE_ON 0x00000080

#define ADDR_FILTER 0x0302
#define LEN_FILTER 2
#define FILTER_OK 0x00
#define FILTER_BAD 0x01

#define U0005_LEN 2
#define U0005_VAL 0x7E

#define U0007_LEN 2
#define U0007_VAL 0x00

#define U0008_LEN 2
#define U0008_VAL 0x00

#define U0009_LEN 2
#define U0009_VAL 0x00

#define U000A_LEN 2
#define U000A_VAL 0x00

#define U0011_LEN 2
#define U0011_VAL 0xFF

#define U0012_LEN 2
#define U0012_VAL 0xFF

#define U0013_LEN 2
#define U0013_VAL 0x03

#define U0014_LEN 2
#define U0014_VAL 0x00

#define U0101_LEN 2
#define U0101_VAL 0x7E

#define U0201_LEN 4
#define U0201_VAL 0x0000



typedef struct HiConfig HiConfig;
struct HiConfig{
  int power;     // 0000-RW: Power state 0=OFF 1=ON
  int mode;      // 0001-RW: Operation mode 0x0010=hot 0x0020=dry 0x0040=cool 0x0050=fan 0x8000=auto
  int speed;     // 0002-RW: Fan speed 0=auto 1=high 2=medium 3=low 4=Silent
  int target;    // 0003-RW: Target termperature in °C
  int u0005;     // 0005-RO: unknown, always 0x7E=0b01111110
  int permission;// 0006-RO: Control via remote control permission, 00 = all settings allowed 01 = all settings prohibited (not tested !)
  int u0007;     // 0007-RO: unknown, always 00
  int u0008;     // 0008-RO: unknown, always 00
  int u0009;     // 0009-RO: unknown, always 00
  int u000A;     // 000A-RO: unknown, always 00
  int u0011;     // 0011-RO: unknown, always FF
  int u0012;     // 0012-RO: unknown, always FF
  int u0013;     // 0013-RO: unknown, always 03
  int u0014;     // 0014-RO: unknown, always 00
  int indoor;    // 0100-RO: Current indoor temperature in °C
  int u0101;     // 0101-RO: unknown, always 0x7E=0b01111110
  int outdoor;   // 0102-RO: Current outdoor temperature in °C
  int u0201;     // 0201-RO: unknown, always 0000
  int u0300;     // 0300-WO: unknown, write only, 0000=normal 0040=vacation 
  int active;    // 0301-RO: Active status 0000=Stand-by FFFF=Active
  int filter;    // 0302-RO: Filter status 0=OK 1=BAD
  int absence;   // 0304-RO: Absence 0=disabled 80=activated
  int beep;      // 0800-WO: Beep (TBC), write only, 00=no beep 07=beep
  char sn[21];   // 0900-RO: serial number (TBC), always 5241442D323551504220 for me
};

int hiReadAll(HiConfig* config);
int hiSetAll(HiConfig* config);

// Read parameter from HVAC
int hiRead(int address, char* data);
int hiRead(int address, int* result);

void printConfig(HiConfig* config);

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
