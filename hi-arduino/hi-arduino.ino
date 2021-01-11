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
  hiReceiveST(buf);
  
  


}
