#include <WIFI/wifi_functions.h>

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println(F("[WiFi] WiFi connected"));
    Serial.println(F("[WiFi] Obtained IP address: "));
    Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));

    // Upper layers (NTP + MQTT) are started only when IP is available.
    WiFiNetworkReady();
}