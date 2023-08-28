#include <MQTT/mqtt_functions.h>

extern const char* mqttServer;

// begin connection to MQTT broker
void connectToMqtt()
{
  Serial.print(F("Connecting to MQTT broker "));
  Serial.println(mqttServer);
  mqttClient.connect();
}