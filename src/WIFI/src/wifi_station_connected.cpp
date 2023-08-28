#include <WIFI/wifi_functions.h>
#include <HWCONFIG/hwConfig.h>

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    digitalWrite(pinWiFiConnected, HIGH);
    Serial.println(F("Connected to AP successfully!"));
}