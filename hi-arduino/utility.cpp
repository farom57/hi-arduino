#include "utility.h"
#include <stdio.h>
#include <stdarg.h>
#include "Arduino.h"

// Classic printf function output to Serial
void serialPrintf(char *fmt, ... ) {
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(buf, 128, fmt, args);
  va_end (args);
  Serial.print(buf);
}
