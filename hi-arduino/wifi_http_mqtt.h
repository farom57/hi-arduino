#ifndef WIFI_HTTP_MQTT_H
#define WIFI_HTTP_MQTT_H
#include "wifi_defines.h"


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



void httpInit();
void httpHandleClient();
void handleForm();
void handleNotFound();
void handleRoot();

void mqttInit();
void mqttHandleClient();

#endif // WIFI_HTTP_MQTT_H
