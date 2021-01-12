#include "hi_arduino.h"
#include <stdio.h>
#include <stdarg.h>


void setup() {
  Serial.begin(9600);

  Serial1.begin(9600, SERIAL_8O1);
  Serial1.setTimeout(200);

  pinMode(LED_BUILTIN, OUTPUT);
  delay(2000);
  Serial.println("Started");
}



int valid_addr[22] = {
  0x0000,
  0x0001,
  0x0002,
  0x0003,
  0x0005,
  0x0006,
  0x0007,
  0x0008,
  0x0009,
  0x000A,
  0x0011,
  0x0012,
  0x0013,
  0x0014,
  0x0100,
  0x0101,
  0x0102,
  0x0201,
  0x0301,
  0x0302,
  0x0304,
  0x0900
};

void loop() {
  char buf[MAX_LENGTH];
  for (int i = 20; i < 26; i++) {
    Serial.print("Set T=");
    Serial.println(i);
    snprintf(buf, 10, "%04X", i);
    delay(20000);
    int res = hiSet(0x0003, buf, 4);
    delay(20000);
    
  }
  Serial.println();





}
