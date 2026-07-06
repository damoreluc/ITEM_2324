#include <WIFI/wifi_functions.h>
#include <MQTT/mqtt_functions.h>

// Operations to perform on upper layers when IP layer is down
void WiFiNetworkFail() {
    // stop MQTT connection monitoring while 
    // WiFi connection is being restored
    stopTimersRTOS();
}