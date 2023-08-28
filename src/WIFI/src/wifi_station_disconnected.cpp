#include <WIFI/wifi_functions.h>
#include <HWCONFIG/hwConfig.h>

// tabella dei codici reason in:
// https://community.cisco.com/t5/wireless-mobility-knowledge-base/802-11-association-status-802-11-deauth-reason-codes/ta-p/3148055
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.print(F("WiFi lost connection. Reason: "));
    Serial.println(info.wifi_sta_disconnected.reason);
    Serial.println(F("Trying to Reconnect"));
    WiFi.reconnect();
    //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}