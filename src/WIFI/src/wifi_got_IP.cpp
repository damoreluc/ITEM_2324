#include <WIFI/wifi_functions.h>

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println(F("WiFi connected"));
    Serial.println(F("Obtained IP address: "));
    Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
}