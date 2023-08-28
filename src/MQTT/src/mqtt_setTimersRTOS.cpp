#include <MQTT/mqtt_functions.h>

// set RTOS timer to handle automatic reconnection to MQTT broker
void setTimersRTOS(uint16_t timeout_ms)
{
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(timeout_ms), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
}