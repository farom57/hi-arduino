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

const char form[] ="<!DOCTYPE HTML>\n\
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
    <form action=\"/form.html\">\n\
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
  server.on(F("/"), handleForm);
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
    server.send(200, F("text/plain"), "POST body was:\n" + server.arg("plain"));
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

// Classic serialPrintf function output to Client
void clientPrintf(WiFiClient client, char *fmt, ... ) {
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(buf, 128, fmt, args);
  va_end (args);
  client.print(buf);
}

void handleRoot(){

  char power_txt[64];
  char mode_txt[64];
  char speed_txt[64];
  char active_txt[64];
  char permission_txt[64];
  char filter_txt[64];
  char absence_txt[64];
  char u0005_txt[64]="TODO";
  char u0007_txt[64]="TODO";
  char u0008_txt[64]="TODO";
  char u0009_txt[64]="TODO";
  char u000A_txt[64]="TODO";
  char u0011_txt[64]="TODO";
  char u0012_txt[64]="TODO";
  char u0013_txt[64]="TODO";
  char u0014_txt[64]="TODO";
  char u0101_txt[64]="TODO";
  char u0201_txt[64]="TODO";
  char buf[2500];

  Serial.print("0 ");
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
  Serial.print("1 ");
  switch (config.mode) {
    case MODE_HOT:
      strcpy(mode_txt,"HOT");
      break;
    case MODE_DRY:
       strcpy(mode_txt,"DRY");
      break;
    case MODE_COOL:
       strcpy(mode_txt,"COOL");
      break;
    case MODE_FAN:
       strcpy(mode_txt,"FAN");
      break;
    case MODE_AUTO:
       strcpy(mode_txt,"AUTO");
      break;
    default:
      snprintf(mode_txt, 64, "unknown value 0x%04X", config.mode);
  }
  Serial.print("2 ");
  switch (config.speed) {
    case SPEED_AUTO:
      strcpy(speed_txt,"AUTO");
      break;
    case SPEED_HIGH:
      strcpy(speed_txt,"HIGH");
      break;
    case SPEED_MEDIUM:
      strcpy(speed_txt,"MEDIUM");
      break;
    case SPEED_LOW:
      strcpy(speed_txt,"LOW");
      break;
    case SPEED_SILENT:
      strcpy(speed_txt,"SILENT");
      break;
    default:
      snprintf(speed_txt, 64, "unknown value 0x%02X", config.speed);
  }
  Serial.print("3 ");
  switch (config.active) {
    case ACTIVE_ON:
      strcpy(active_txt,"ON");
      break;
    case ACTIVE_OFF:
      strcpy(active_txt,"OFF");
      break;
    default:
      snprintf(active_txt, 64, "unknown value 0x%04X", config.active);
  }
  Serial.print("4 ");
  switch (config.permission) {
    case PERMISSION_ALLOWED:
      strcpy(permission_txt,"ALLOWED");
      break;
    case PERMISSION_PROHIBITED:
      strcpy(permission_txt,"PROHIBITED");
      break;
    default:
      snprintf(permission_txt, 64, "unknown value 0x%02X", config.permission);
  }
  Serial.print("5 ");
  switch (config.filter) {
    case FILTER_OK:
      strcpy(filter_txt,"OK");
      break;
    case FILTER_BAD:
      strcpy(filter_txt,"BAD");
      break;
    default:
      snprintf(filter_txt, 64, "unknown value 0x%02X", config.filter);
  }
  Serial.print("6 ");
  switch (config.absence) {
    case ABSENCE_OFF:
      strcpy(absence_txt,"OFF");
      break;
    case ABSENCE_ON:
      strcpy(absence_txt,"ON");
      break;
    default:
      snprintf(absence_txt, 64, "unknown value 0x%08X", config.absence);
  }
  Serial.print("7 ");
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
    (config.power==POWER_ON?"checked=\"checked\"":""),
    (config.power==POWER_OFF?"checked=\"checked\"":""),
    (config.mode==MODE_HOT?"checked=\"checked\"":""),
    (config.mode==MODE_DRY?"checked=\"checked\"":""),
    (config.mode==MODE_COOL?"checked=\"checked\"":""),
    (config.mode==MODE_FAN?"checked=\"checked\"":""),
    (config.mode==MODE_AUTO?"checked=\"checked\"":""),
    (config.speed==SPEED_AUTO?"checked=\"checked\"":""),
    (config.speed==SPEED_HIGH?"checked=\"checked\"":""),
    (config.speed==SPEED_MEDIUM?"checked=\"checked\"":""),
    (config.speed==SPEED_LOW?"checked=\"checked\"":""),
    (config.speed==SPEED_SILENT?"checked=\"checked\"":""),

    config.target
  );
  Serial.print("8 ");
  server.send(200, F("text/html"), buf);
  Serial.print("9 ");
  /*clientPrintf(client, "Unknown parameter at 0x0005: 0x%02X ", config.u0005);
  if (config.u0005 == U0005_VAL)
    clientPrintf(client, "OK");
  else
    clientPrintf(client, "Different from typical value 0x%02X", U0005_VAL);
  client.println("<br />");

  clientPrintf(client, "Unknown parameter at 0x0007: 0x%02X ", config.u0007);
  if (config.u0007 == U0007_VAL)
    clientPrintf(client, "OK");
  else
    clientPrintf(client, "Different from typical value 0x%02X", U0007_VAL);
  client.println("<br />");

  clientPrintf(client, "Unknown parameter at 0x0008: 0x%02X ", config.u0008);
  if (config.u0008 == U0008_VAL)
    clientPrintf(client, "OK");
  else
    clientPrintf(client, "Different from typical value 0x%02X", U0008_VAL);
  client.println("<br />");

  clientPrintf(client, "Unknown parameter at 0x0009: 0x%02X ", config.u0009);
  if (config.u0009 == U0009_VAL)
    clientPrintf(client, "OK");
  else
    clientPrintf(client, "Different from typical value 0x%02X", U0009_VAL);
  client.println("<br />");

  clientPrintf(client, "Unknown parameter at 0x000A: 0x%02X ", config.u000A);
  if (config.u000A == U000A_VAL)
    clientPrintf(client, "OK");
  else
    clientPrintf(client, "Different from typical value 0x%02X", U000A_VAL);
  client.println("<br />");

  clientPrintf(client, "Unknown parameter at 0x0011: 0x%02X ", config.u0011);
  if (config.u0011 == U0011_VAL)
    clientPrintf(client, "OK");
  else
    clientPrintf(client, "Different from typical value 0x%02X", U0011_VAL);
  client.println("<br />");

  clientPrintf(client, "Unknown parameter at 0x0012: 0x%02X ", config.u0012);
  if (config.u0012 == U0012_VAL)
    clientPrintf(client, "OK");
  else
    clientPrintf(client, "Different from typical value 0x%02X", U0012_VAL);
  client.println("<br />");

  clientPrintf(client, "Unknown parameter at 0x0013: 0x%02X ", config.u0013);
  if (config.u0013 == U0013_VAL)
    clientPrintf(client, "OK");
  else
    clientPrintf(client, "Different from typical value 0x%02X", U0013_VAL);
  client.println("<br />");

  clientPrintf(client, "Unknown parameter at 0x0014: 0x%02X ", config.u0014);
  if (config.u0014 == U0014_VAL)
    clientPrintf(client, "OK");
  else
    clientPrintf(client, "Different from typical value 0x%02X", U0014_VAL);
  client.println("<br />");

  clientPrintf(client, "Unknown parameter at 0x0101: 0x%02X ", config.u0101);
  if (config.u0101 == U0101_VAL)
    clientPrintf(client, "OK");
  else
    clientPrintf(client, "Different from typical value 0x%02X", U0101_VAL);
  client.println("<br />");

  clientPrintf(client, "Unknown parameter at 0x0201: 0x%02X ", config.u0201);
  if (config.u0201 == U0201_VAL)
    clientPrintf(client, "OK");
  else
    clientPrintf(client, "Different from typical value 0x%02X", U0201_VAL);
  client.println("<br />");


  client.println("<h3>Change parameters:</h3>");
  client.println("<form action=\"/\">");
  client.println("<label for=\"power\">Power:</label> ");
  client.println("<input checked=\"checked\" name=\"power\" type=\"radio\" value=\"ON\" />ON ");
  client.println("<input name=\"power\" type=\"radio\" value=\"OFF\" />OFF<br />");
  client.println("<label for=\"mode\">Mode:</label> ");
  client.println("<input checked=\"checked\" name=\"mode\" type=\"radio\" value=\"HOT\" />HOT ");
  client.println("<input name=\"mode\" type=\"radio\" value=\"DRY\" />DRY ");
  client.println("<input name=\"mode\" type=\"radio\" value=\"COOL\" />COOL ");
  client.println("<input name=\"mode\" type=\"radio\" value=\"FAN\" />FAN ");
  client.println("<input name=\"mode\" type=\"radio\" value=\"AUTO\" />AUTO<br />");
  client.println("<label for=\"speed\">Fan speed:</label> ");
  client.println("<input checked=\"checked\" name=\"speed\" type=\"radio\" value=\"AUTO\" />AUTO ");
  client.println("<input name=\"speed\" type=\"radio\" value=\"HIGH\" />HIGH ");
  client.println("<input  name=\"speed\" type=\"radio\" value=\"MEDIUM\" />MEDIUM ");
  client.println("<input  name=\"speed\" type=\"radio\" value=\"LOW\" />LOW ");
  client.println("<input  name=\"speed\" type=\"radio\" value=\"SILENT\" />SILENT<br />");
  client.println("<label for=\"target\">Target temperature:</label> ");
  client.println("<input max=\"32\" min=\"16\" name=\"target\" type=\"number\" value=\"20\"/><br />");
  client.println("<input type=\"submit\" /><br />");
  client.println("</form>");
  client.println("</body>");
  client.println("</html>");*/
}
