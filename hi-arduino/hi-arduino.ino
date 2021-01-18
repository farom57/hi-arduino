#include "hi_arduino.h"
#include "secrets.h"
#include <stdio.h>
#include <stdarg.h>

#include <WiFiNINA.h>
///////please enter your sensitive data in the Secret tab/secrets.h
char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;           // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;

WiFiServer server(80);
HiConfig config;
const long interval = 10000;
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);

  Serial1.begin(9600, SERIAL_8O1);
  Serial1.setTimeout(200);

  pinMode(LED_BUILTIN, OUTPUT);
  delay(2000);
  Serial.println("Started");

  initWifiWeb();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    hiReadAll(&config);
    printConfig(&config);
  }

  manageWifiWeb();
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
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();
}

void manageWifiWeb() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<h2>HI-LINK Arduino</h2>");
          client.println("<h3>Current status:</h3>");

          client.print("Power: ");
          switch (config.power) {
            case POWER_ON:
              client.print("ON");
              break;
            case POWER_OFF:
              client.print("OFF");
              break;
            default:
              clientPrintf(client, "unknown value 0x%02X", config.power);
          }
          client.println("<br />");

          client.print("Mode: ");
          switch (config.mode) {
            case MODE_HOT:
              client.print("HOT");
              break;
            case MODE_DRY:
              client.print("DRY");
              break;
            case MODE_COOL:
              client.print("COOL");
              break;
            case MODE_FAN:
              client.print("FAN");
              break;
            case MODE_AUTO:
              client.print("AUTO");
              break;
            default:
              clientPrintf(client, "unknown value 0x%04X", config.mode);
          }
          client.println("<br />");

          client.print("Fan Speed: ");
          switch (config.speed) {
            case SPEED_AUTO:
              client.print("AUTO");
              break;
            case SPEED_HIGH:
              client.print("HIGH");
              break;
            case SPEED_MEDIUM:
              client.print("MEDIUM");
              break;
            case SPEED_LOW:
              client.print("LOW");
              break;
            case SPEED_SILENT:
              client.print("SILENT");
              break;
            default:
              clientPrintf(client, "unknown value 0x%02X", config.speed);
          }
          client.println("<br />");

          clientPrintf(client, "Target temperature: %d&deg;C<br />\n", config.target);

          clientPrintf(client, "Current indoor temperature: %d&deg;C<br />\n", config.indoor);

          clientPrintf(client, "Current outdoor temperature: %d&deg;C<br />\n", config.outdoor);

          client.print("Active: ");
          switch (config.active) {
            case ACTIVE_ON:
              client.print("ON");
              break;
            case ACTIVE_OFF:
              client.print("OFF");
              break;
            default:
              clientPrintf(client, "unknown value 0x%04X", config.active);
          }
          client.println("<br />");

          client.print("Control via remote control: ");
          switch (config.permission) {
            case PERMISSION_ALLOWED:
              client.print("ALLOWED");
              break;
            case PERMISSION_PROHIBITED:
              client.print("PROHIBITED");
              break;
            default:
              clientPrintf(client, "unknown value 0x%02X", config.permission);
          }
          client.println("<br />");

          client.print("Filter status: ");
          switch (config.filter) {
            case FILTER_OK:
              client.print("OK");
              break;
            case FILTER_BAD:
              client.print("BAD");
              break;
            default:
              clientPrintf(client, "unknown value 0x%02X", config.filter);
          }
          client.println("<br />");

          client.print("Absence mode: ");
          switch (config.absence) {
            case ABSENCE_OFF:
              client.print("OFF");
              break;
            case ABSENCE_ON:
              client.print("ON");
              break;
            default:
              clientPrintf(client, "unknown value 0x%08X", config.absence);
          }
          client.println("<br />");

          clientPrintf(client, "Serial:  %s<br />\n", config.sn);

          clientPrintf(client, "Unknown parameter at 0x0005: 0x%02X ", config.u0005);
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
          client.println("<form action=\"/form.html\">");
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
          client.println("<form />");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
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
