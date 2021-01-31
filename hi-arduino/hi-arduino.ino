#include "hi_link.h"
#include "secrets.h"
#include "wifi_http_mqtt.h"

const long interval = 10000;
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Starting");

  hiInit();
  wifiInit();
  httpInit();

  Serial.println("Ready");

}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    hiReadAll(&config);
    //printConfig(&config);
    mqttUpdate(&config);
  }

  httpHandleClient();
  mqttReconnect();

  //delay(100);
}
