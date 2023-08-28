#include <MQTT/mqtt_functions.h>

// ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
void stopTimersRTOS()
{
    // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
    xTimerStop(mqttReconnectTimer, 0);
}
