#include "wifi_defines.h"
#include "Arduino.h"
#include "wifi_http_mqtt.h"
#include "hi_link.h"

#include <WiFiWebServer.h>
#include <PubSubClient.h>

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;     // the Wifi radio's status
int reqCount = 0;                // number of requests received

WiFiWebServer server(80);

const char* mqttServer = "192.168.1.100";        // Broker address
const char *ID        = "Hi-Arduino";  // Name of our device, must be unique
const char *TOPIC     = "MQTT_Pub";               // Topic to subcribe to
const char *subTopic  = "MQTT_Sub";               // Topic to subcribe to


void wifiInit(){
    // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  printWifiStatus();
}



void httpInit() {
  server.on(F("/"), handleRoot);
  server.on(F("/form"), handleForm);
  server.onNotFound(handleNotFound);
  server.begin();  
}

void httpHandleClient(){
  server.handleClient();
}



void handleForm()
{
  if (server.method() != HTTP_POST)
  {
    server.send(405, F("text/plain"), F("Method Not Allowed"));
  } else
  {

    String msg = "<html><head><meta http-equiv=\"refresh\" content=\"10;url=/\"/></head>Command received:<br/>";
    bool ok = true;

    msg += "Power: ";
    if (server.arg("power") == "ON") {
      msg += "ON<br/>";
      config.power = POWER_ON;
    } else if (server.arg("power") == "OFF") {
      msg += "OFF<br/>";
      config.power = POWER_OFF;
    } else {
      msg += "Unknown value<br/>Error, incorrect value, command not executed";
      ok = false;
    }


    msg += "Mode: ";
    if (server.arg("mode") == "HOT") {
      msg += "HOT<br/>";
      config.mode = MODE_HOT;
    } else if (server.arg("mode") == "DRY") {
      msg += "DRY<br/>";
      config.mode = MODE_DRY;
    } else if (server.arg("mode") == "COOL") {
      msg += "COOL<br/>";
      config.mode = MODE_COOL;
    } else if (server.arg("mode") == "FAN") {
      msg += "FAN<br/>";
      config.mode = MODE_FAN;
    } else if (server.arg("mode") == "AUTO") {
      msg += "AUTO<br/>";
      config.mode = MODE_AUTO;
    } else {
      msg += "Unknown value<br/>Error, incorrect value, command not executed";
      ok = false;
    }


    msg += "Speed: ";
    if (server.arg("speed") == "AUTO") {
      msg += "AUTO<br/>";
      config.speed = SPEED_AUTO;
    } else if (server.arg("speed") == "HIGH") {
      msg += "HIGH<br/>";
      config.speed = SPEED_HIGH;
    } else if (server.arg("speed") == "MEDIUM") {
      msg += "MEDIUM<br/>";
      config.speed = SPEED_MEDIUM;
    } else if (server.arg("speed") == "LOW") {
      msg += "LOW<br/>";
      config.speed = SPEED_LOW;
    } else if (server.arg("speed") == "SILENT") {
      msg += "SILENT<br/>";
      config.speed = SPEED_SILENT;
    } else {
      msg += "Unknown value<br/>Error, incorrect value, command not executed";
      ok = false;
    }

    int val = server.arg("target").toInt();
    msg += "Target: ";
    if (val >= TARGET_MIN && val <= TARGET_MAX) {
      msg += String(val) + "degC<br/>";
      config.target = val;
    } else {
      msg += server.arg("target") + "Incorrect value<br/>Error, incorrect value, command not executed";
      ok = false;
    }

    if (ok) {
      int res = hiSetAll(&config);
      msg += "Res: " + String(res);
    }
    server.send(200, F("text/html"), msg);
  }
}

void handleNotFound()
{

  String message = F("File Not Found\n\n");

  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? F("GET") : F("POST");
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, F("text/plain"), message);


}

void handleRoot() {

  char power_txt[64];
  char mode_txt[64];
  char speed_txt[64];
  char active_txt[64];
  char permission_txt[64];
  char filter_txt[64];
  char absence_txt[64];
  char u0005_txt[64];
  char u0007_txt[64];
  char u0008_txt[64];
  char u0009_txt[64];
  char u000A_txt[64];
  char u0011_txt[64];
  char u0012_txt[64];
  char u0013_txt[64];
  char u0014_txt[64];
  char u0101_txt[64];
  char u0201_txt[64];
  char buf[2500];


  switch (config.power) {
    case POWER_ON:
      strcpy(power_txt, "ON");
      break;
    case POWER_OFF:
      strcpy(power_txt, "OFF");
      break;
    default:
      snprintf(power_txt, 64, "unknown value 0x%02X", config.power);
  }

  switch (config.mode) {
    case MODE_HOT:
      strcpy(mode_txt, "HOT");
      break;
    case MODE_DRY:
      strcpy(mode_txt, "DRY");
      break;
    case MODE_COOL:
      strcpy(mode_txt, "COOL");
      break;
    case MODE_FAN:
      strcpy(mode_txt, "FAN");
      break;
    case MODE_AUTO:
      strcpy(mode_txt, "AUTO");
      break;
    default:
      snprintf(mode_txt, 64, "unknown value 0x%04X", config.mode);
  }

  switch (config.speed) {
    case SPEED_AUTO:
      strcpy(speed_txt, "AUTO");
      break;
    case SPEED_HIGH:
      strcpy(speed_txt, "HIGH");
      break;
    case SPEED_MEDIUM:
      strcpy(speed_txt, "MEDIUM");
      break;
    case SPEED_LOW:
      strcpy(speed_txt, "LOW");
      break;
    case SPEED_SILENT:
      strcpy(speed_txt, "SILENT");
      break;
    default:
      snprintf(speed_txt, 64, "unknown value 0x%02X", config.speed);
  }

  switch (config.active) {
    case ACTIVE_ON:
      strcpy(active_txt, "ON");
      break;
    case ACTIVE_OFF:
      strcpy(active_txt, "OFF");
      break;
    default:
      snprintf(active_txt, 64, "unknown value 0x%04X", config.active);
  }

  switch (config.permission) {
    case PERMISSION_ALLOWED:
      strcpy(permission_txt, "ALLOWED");
      break;
    case PERMISSION_PROHIBITED:
      strcpy(permission_txt, "PROHIBITED");
      break;
    default:
      snprintf(permission_txt, 64, "unknown value 0x%02X", config.permission);
  }

  switch (config.filter) {
    case FILTER_OK:
      strcpy(filter_txt, "OK");
      break;
    case FILTER_BAD:
      strcpy(filter_txt, "BAD");
      break;
    default:
      snprintf(filter_txt, 64, "unknown value 0x%02X", config.filter);
  }

  switch (config.absence) {
    case ABSENCE_OFF:
      strcpy(absence_txt, "OFF");
      break;
    case ABSENCE_ON:
      strcpy(absence_txt, "ON");
      break;
    default:
      snprintf(absence_txt, 64, "unknown value 0x%08X", config.absence);
  }

  if (config.u0005 == U0005_VAL)
    snprintf(u0005_txt, 64, "0x%02X OK", config.u0005);
  else
    snprintf(u0005_txt, 64, "0x%02X Different from typical value 0x%02X", config.u0005, U0005_VAL);

  if (config.u0007 == U0007_VAL)
    snprintf(u0007_txt, 64, "0x%02X OK", config.u0007);
  else
    snprintf(u0007_txt, 64, "0x%02X Different from typical value 0x%02X", config.u0007, U0007_VAL);

  if (config.u0008 == U0008_VAL)
    snprintf(u0008_txt, 64, "0x%02X OK", config.u0008);
  else
    snprintf(u0008_txt, 64, "0x%02X Different from typical value 0x%02X", config.u0008, U0008_VAL);

  if (config.u0009 == U0009_VAL)
    snprintf(u0009_txt, 64, "0x%02X OK", config.u0009);
  else
    snprintf(u0009_txt, 64, "0x%02X Different from typical value 0x%02X", config.u0009, U0009_VAL);

  if (config.u000A == U000A_VAL)
    snprintf(u000A_txt, 64, "0x%02X OK", config.u000A);
  else
    snprintf(u000A_txt, 64, "0x%02X Different from typical value 0x%02X", config.u000A, U000A_VAL);

  if (config.u0011 == U0011_VAL)
    snprintf(u0011_txt, 64, "0x%02X OK", config.u0011);
  else
    snprintf(u0011_txt, 64, "0x%02X Different from typical value 0x%02X", config.u0011, U0011_VAL);

  if (config.u0012 == U0012_VAL)
    snprintf(u0012_txt, 64, "0x%02X OK", config.u0012);
  else
    snprintf(u0012_txt, 64, "0x%02X Different from typical value 0x%02X", config.u0012, U0012_VAL);

  if (config.u0013 == U0013_VAL)
    snprintf(u0013_txt, 64, "0x%02X OK", config.u0013);
  else
    snprintf(u0013_txt, 64, "0x%02X Different from typical value 0x%02X", config.u0013, U0013_VAL);

  if (config.u0014 == U0014_VAL)
    snprintf(u0014_txt, 64, "0x%02X OK", config.u0014);
  else
    snprintf(u0014_txt, 64, "0x%02X Different from typical value 0x%02X", config.u0014, U0014_VAL);

  if (config.u0101 == U0101_VAL)
    snprintf(u0101_txt, 64, "0x%02X OK", config.u0101);
  else
    snprintf(u0101_txt, 64, "0x%02X Different from typical value 0x%02X", config.u0101, U0101_VAL);

  if (config.u0201 == U0201_VAL)
    snprintf(u0201_txt, 64, "0x%04X OK", config.u0201);
  else
    snprintf(u0201_txt, 64, "0x%04X Different from typical value 0x%04X", config.u0201, U0201_VAL);

  snprintf(buf, 2500, form,
           power_txt,
           mode_txt,
           speed_txt,
           config.target,
           config.indoor,
           config.outdoor,
           active_txt,
           permission_txt,
           filter_txt,
           absence_txt,
           config.sn,
           u0005_txt,
           u0007_txt,
           u0008_txt,
           u0009_txt,
           u000A_txt,
           u0011_txt,
           u0012_txt,
           u0013_txt,
           u0014_txt,
           u0101_txt,
           u0201_txt,
           (config.power == POWER_ON ? "checked=\"checked\"" : ""),
           (config.power == POWER_OFF ? "checked=\"checked\"" : ""),
           (config.mode == MODE_HOT ? "checked=\"checked\"" : ""),
           (config.mode == MODE_DRY ? "checked=\"checked\"" : ""),
           (config.mode == MODE_COOL ? "checked=\"checked\"" : ""),
           (config.mode == MODE_FAN ? "checked=\"checked\"" : ""),
           (config.mode == MODE_AUTO ? "checked=\"checked\"" : ""),
           (config.speed == SPEED_AUTO ? "checked=\"checked\"" : ""),
           (config.speed == SPEED_HIGH ? "checked=\"checked\"" : ""),
           (config.speed == SPEED_MEDIUM ? "checked=\"checked\"" : ""),
           (config.speed == SPEED_LOW ? "checked=\"checked\"" : ""),
           (config.speed == SPEED_SILENT ? "checked=\"checked\"" : ""),
           config.target
          );

  server.send(200, F("text/html"), buf);


}
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void mqttInit(){

}

void mqttHandleClient(){
  
}
