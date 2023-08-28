#include <MQTT/mqtt_functions.h>

// FreeRTOS timer to force MQTT reconnection when WiFi is connected
TimerHandle_t mqttReconnectTimer;