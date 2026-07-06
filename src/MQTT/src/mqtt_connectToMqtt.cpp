#include <MQTT/mqtt_functions.h>

// begin connection to MQTT broker
void connectToMqtt()
{
  Serial.println(F("Connecting to MQTT broker"));
  if (getSensMode() == REAL_DATA)
  {
    ssd1306_publish("Connecting to MQTT\n");
  }
  mqttClient.connect();
}