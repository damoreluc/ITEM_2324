#include <MQTT/mqtt_functions.h>

// handle onMqttPublish event: fired when message publish is acknowledged - optional
//   note: publish your message calling mqttClient.publish method
// fired by mqttClient.onPublish on publishing to MQTT broker
void onMqttPublish(uint16_t packetId)
{
  Serial.println(F("Publish acknowledged."));
  Serial.print(F("  packetId: "));
  Serial.println(packetId);
}