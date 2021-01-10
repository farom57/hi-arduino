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





void loop() {
  char buf[MAX_LENGTH];
  int temp=0;
  int err = hiRead(0x0003, &temp);
  Serial.print("T=");Serial.print(temp);Serial.println("°C");

  delay(2000);

}
