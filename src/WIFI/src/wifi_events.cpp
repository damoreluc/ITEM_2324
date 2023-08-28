

/* WiFi Events

0  ARDUINO_EVENT_WIFI_READY                   < ESP32 WiFi ready
1  ARDUINO_EVENT_WIFI_SCAN_DONE               < ESP32 finish scanning AP
2  ARDUINO_EVENT_WIFI_STA_START               < ESP32 station start
3  ARDUINO_EVENT_WIFI_STA_STOP                < ESP32 station stop
4  ARDUINO_EVENT_WIFI_STA_CONNECTED           < ESP32 station connected to AP
5  ARDUINO_EVENT_WIFI_STA_DISCONNECTED        < ESP32 station disconnected from AP
6  ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE     < the auth mode of AP connected by ESP32 station changed
7  ARDUINO_EVENT_WIFI_STA_GOT_IP              < ESP32 station got IP from connected AP
8  ARDUINO_EVENT_WIFI_STA_LOST_IP             < ESP32 station lost IP and the IP is reset to 0
9  ARDUINO_EVENT_WPS_ER_SUCCESS               < ESP32 station wps succeeds in enrollee mode
10 ARDUINO_EVENT_WPS_ER_FAILED                < ESP32 station wps fails in enrollee mode
11 ARDUINO_EVENT_WPS_ER_TIMEOUT               < ESP32 station wps timeout in enrollee mode
12 ARDUINO_EVENT_WPS_ER_PIN                   < ESP32 station wps pin code in enrollee mode
13 ARDUINO_EVENT_WIFI_AP_START                < ESP32 soft-AP start
14 ARDUINO_EVENT_WIFI_AP_STOP                 < ESP32 soft-AP stop
15 ARDUINO_EVENT_WIFI_AP_STACONNECTED         < a station connected to ESP32 soft-AP
16 ARDUINO_EVENT_WIFI_AP_STADISCONNECTED      < a station disconnected from ESP32 soft-AP
17 ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED        < ESP32 soft-AP assign an IP to a connected station
18 ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED       < Receive probe request packet in soft-AP interface
19 ARDUINO_EVENT_WIFI_AP_GOT_IP6              < ESP32 ap interface v6IP addr is preferred
19 ARDUINO_EVENT_WIFI_STA_GOT_IP6             < ESP32 station interface v6IP addr is preferred
20 ARDUINO_EVENT_ETH_START                    < ESP32 ethernet start
21 ARDUINO_EVENT_ETH_STOP                     < ESP32 ethernet stop
22 ARDUINO_EVENT_ETH_CONNECTED                < ESP32 ethernet phy link up
23 ARDUINO_EVENT_ETH_DISCONNECTED             < ESP32 ethernet phy link down
24 ARDUINO_EVENT_ETH_GOT_IP                   < ESP32 ethernet got IP from connected AP
19 ARDUINO_EVENT_ETH_GOT_IP6                  < ESP32 ethernet interface v6IP addr is preferred
25 ARDUINO_EVENT_MAX
*/

#include <WIFI/wifi_functions.h>
#include <HWCONFIG/hwConfig.h>

void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);

    switch (event)
    {
    case ARDUINO_EVENT_WIFI_READY:
        Serial.println(F("WiFi interface ready"));
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        Serial.println(F("Completed scan for access points"));
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        Serial.println(F("WiFi client started"));
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        Serial.println(F("WiFi clients stopped"));
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        digitalWrite(pinWiFiConnected, HIGH);
        Serial.print(F("Connected to access point "));
        Serial.print(WiFi.SSID());
        Serial.print(F("  (RSSI: "));
        Serial.print(WiFi.RSSI());
        Serial.println(")");
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        digitalWrite(pinWiFiConnected, LOW);
        Serial.println(F("Disconnected from WiFi access point"));
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        Serial.println(F("Authentication mode of access point has changed"));
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print(F("Obtained IP address: "));
        Serial.println(WiFi.localIP());
        // operazioni da compiere sui layers superiori dopo che il layer IP è pronto
        WiFiNetworkReady();
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        Serial.println(F("Lost IP address and IP address is reset to 0"));
        // operazioni da compiere sui layers superiori se il layer IP è caduto
        WiFiNetworkFail();
        break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
        Serial.println(F("WiFi Protected Setup (WPS): succeeded in enrollee mode"));
        break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
        Serial.println(F("WiFi Protected Setup (WPS): failed in enrollee mode"));
        break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
        Serial.println(F("WiFi Protected Setup (WPS): timeout in enrollee mode"));
        break;
    case ARDUINO_EVENT_WPS_ER_PIN:
        Serial.println(F("WiFi Protected Setup (WPS): pin code in enrollee mode"));
        break;
    case ARDUINO_EVENT_WIFI_AP_START:
        Serial.println(F("WiFi access point started"));
        break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
        Serial.println(F("WiFi access point  stopped"));
        break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        Serial.println(F("Client connected"));
        break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        Serial.println(F("Client disconnected"));
        break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        Serial.println(F("Assigned IP address to client"));
        break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
        Serial.println(F("Received probe request"));
        break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
        Serial.println(F("AP IPv6 is preferred"));
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        Serial.println(F("STA IPv6 is preferred"));
        break;
    case ARDUINO_EVENT_ETH_GOT_IP6:
        Serial.println(F("Ethernet IPv6 is preferred"));
        break;
    case ARDUINO_EVENT_ETH_START:
        Serial.println(F("Ethernet started"));
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println(F("Ethernet stopped"));
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println(F("Ethernet connected"));
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println(F("Ethernet disconnected"));
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.println(F("Obtained IP address"));
        break;
    default:
        break;
    }
}
