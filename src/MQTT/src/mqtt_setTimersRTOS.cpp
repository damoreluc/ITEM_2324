#include <MQTT/mqtt_functions.h>

// set RTOS timer to handle automatic reconnection to MQTT broker
// Timer callback runs on core 0 (PRO_CPU) by default in FreeRTOS
void setTimersRTOS(uint16_t timeout_ms)
{
  // Create timer that will run on core 0 (PRO_CPU)
  // FreeRTOS timers are handled by the timer daemon task which runs on core 0
  mqttReconnectTimer = xTimerCreate(
    "mqttTimer",                                    // name
    pdMS_TO_TICKS(timeout_ms),                      // period in ticks
    pdFALSE,                                        // auto-reload: FALSE (one-shot)
    (void *)0,                                      // timer ID
    reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt) // callback
  );
  
  if (mqttReconnectTimer != NULL) {
    Serial.println(F("MQTT reconnect timer created for core 0 operation"));
  } else {
    Serial.println(F("ERROR: Failed to create MQTT reconnect timer"));
  }
}