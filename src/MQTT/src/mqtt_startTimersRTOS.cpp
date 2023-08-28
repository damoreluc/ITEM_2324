#include <MQTT/mqtt_functions.h>

// start RTOS timer to hancle automatic reconnection to MQTT broker
void startTimersRTOS()
{
    xTimerStart(mqttReconnectTimer, 0);
}
