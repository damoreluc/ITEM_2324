#include <MQTT/mqtt_functions.h>

extern const char* mqttServer;

// begin connection to MQTT broker
void connectToMqtt()
{
  Serial.print(F("Connecting to MQTT broker "));
  Serial.println(mqttServer);
  if (getSensMode() == REAL_DATA)
  {
    ssd1306_publish("Connecting to MQTT\n");
    ssd1306_publish(mqttServer);
  }
  mqttClient.connect();
}