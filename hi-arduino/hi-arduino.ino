#include "hi_arduino.h"
#include "secrets.h"
#include "wifi_defines.h"
#include <stdio.h>
#include <stdarg.h>

//#include <WiFiNINA.h>

WiFiWebServer server(80);
int status = WL_IDLE_STATUS;     // the Wifi radio's status
int reqCount = 0;                // number of requests received

HiConfig config;
const long interval = 10000;
unsigned long previousMillis = 0;

const char form[] = "<!DOCTYPE HTML>\n\
    <html>\n\
    <h2>HI-LINK Arduino</h2>\n\
    <h3>Current status:</h3>\n\
    Power: %s<br />\n\
    Mode: %s<br />\n\
    Fan Speed: %s<br />\n\
    Target temperature: %d&deg;C<br />\n\
    Current indoor temperature: %d&deg;C<br />\n\
    Current outdoor temperature: %d&deg;C<br />\n\
    Active: %s<br />\n\
    Control via remote control: %s<br />\n\
    Filter status: %s<br />\n\
    Absence mode: %s<br />\n\
    Serial:  %s<br />\n\
    Unknown parameter at 0x0005: %s<br />\n\
    Unknown parameter at 0x0007: %s<br />\n\
    Unknown parameter at 0x0008: %s<br />\n\
    Unknown parameter at 0x0009: %s<br />\n\
    Unknown parameter at 0x000A: %s<br />\n\
    Unknown parameter at 0x0011: %s<br />\n\
    Unknown parameter at 0x0012: %s<br />\n\
    Unknown parameter at 0x0013: %s<br />\n\
    Unknown parameter at 0x0014: %s<br />\n\
    Unknown parameter at 0x0101: %s<br />\n\
    Unknown parameter at 0x0201: %s<br />\n\
    <h3>Change parameters:</h3>\n\
    <form method=\"post\" action=\"/form\">\n\
    <label for=\"power\">Power:</label>\n\
    <input %s name=\"power\" type=\"radio\" value=\"ON\" />ON\n\
    <input %s name=\"power\" type=\"radio\" value=\"OFF\" />OFF<br />\n\
    <label for=\"mode\">Mode:</label>\n\
    <input %s name=\"mode\" type=\"radio\" value=\"HOT\" />HOT\n\
    <input %s name=\"mode\" type=\"radio\" value=\"DRY\" />DRY\n\
    <input %s name=\"mode\" type=\"radio\" value=\"COOL\" />COOL\n\
    <input %s name=\"mode\" type=\"radio\" value=\"FAN\" />FAN\n\
    <input %s name=\"mode\" type=\"radio\" value=\"AUTO\" />AUTO<br />\n\
    <label for=\"speed\">Fan speed:</label>\n\
    <input %s name=\"speed\" type=\"radio\" value=\"AUTO\" />AUTO\n\
    <input %s name=\"speed\" type=\"radio\" value=\"HIGH\" />HIGH\n\
    <input %s name=\"speed\" type=\"radio\" value=\"MEDIUM\" />MEDIUM\n\
    <input %s name=\"speed\" type=\"radio\" value=\"LOW\" />LOW\n\
    <input %s name=\"speed\" type=\"radio\" value=\"SILENT\" />SILENT<br />\n\
    <label for=\"target\">Target temperature:</label>\n\
    <input max=\"32\" min=\"16\" name=\"target\" type=\"number\" value=\"%d\"/><br />\n\
    <input type=\"submit\" /><br />\n\
    </form>\n\
    </body>\n\
    </html>";

void setup() {
  Serial.begin(9600);

  Serial1.begin(9600, SERIAL_8O1);
  Serial1.setTimeout(200);

  pinMode(LED_BUILTIN, OUTPUT);
  delay(2000);
  Serial.println("Started");

  initWifiWeb();
}

void initWifiWeb() {
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

  server.on(F("/"), handleRoot);
  server.on(F("/form"), handleForm);
  server.onNotFound(handleNotFound);
  server.begin();

  // you're connected now, so print out the status:
  printWifiStatus();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    hiReadAll(&config);
    printConfig(&config);
  }

  server.handleClient();

}



void handleForm()
{
  if (server.method() != HTTP_POST)
  {
    server.send(405, F("text/plain"), F("Method Not Allowed"));
  } else
  {
    
    String msg = "Command received:\n";
    
    msg += "Power: ";
    if (server.arg("power") == "ON") {
      msg += "ON\n";
      config.power = POWER_ON;
    } else if (server.arg("power") == "OFF") {
      msg += "OFF\n";
      config.power = POWER_OFF;
    } else {
      msg += "Unknown value\nError, incorrect value, command not executed";
      return;
    }

        
    msg += "Mode: ";
    if (server.arg("mode") == "HOT") {
      msg += "HOT\n";
      config.mode = MODE_HOT;
    } else if (server.arg("mode") == "DRY") {
      msg += "DRY\n";
      config.mode = MODE_DRY;
    } else if (server.arg("mode") == "COOL") {
      msg += "COOL\n";
      config.mode = MODE_COOL;
    } else if (server.arg("mode") == "FAN") {
      msg += "FAN\n";
      config.mode = MODE_FAN;
    } else if (server.arg("mode") == "AUTO") {
      msg += "AUTO\n";
      config.mode = MODE_AUTO;
    } else {
      msg += "Unknown value\nError, incorrect value, command not executed";
      return;
    }

        
    msg += "Speed: ";
    if (server.arg("speed") == "AUTO") {
      msg += "AUTO\n";
      config.speed = SPEED_AUTO;
    } else if (server.arg("speed") == "HIGH") {
      msg += "HIGH\n";
      config.speed = SPEED_HIGH;
    } else if (server.arg("speed") == "MEDIUM") {
      msg += "MEDIUM\n";
      config.speed = SPEED_MEDIUM;
    } else if (server.arg("speed") == "LOW") {
      msg += "LOW\n";
      config.speed = SPEED_LOW;
    } else if (server.arg("speed") == "SILENT") {
      msg += "SILENT\n";
      config.speed = SPEED_SILENT;
    } else {
      msg += "Unknown value\nError, incorrect value, command not executed";
      return;
    }

    msg+="Target: ";
    int val;// = server.arg("target").toInt();
    if(val>=TARGET_MIN && val<=TARGET_MAX){
      msg += val + "degC\n";
      config.target=val; 
    }else{
      //Serial.println("rec:");
      //Serial.println(server.arg("target"));
      //Serial.println(val);
      msg += server.arg("target") + " xxx";//"Incorrect value\nError, incorrect value, command not executed";
    }
    server.send(200, F("text/plain"), msg);
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
