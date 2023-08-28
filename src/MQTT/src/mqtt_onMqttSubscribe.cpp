#include <MQTT/mqtt_functions.h>

// handle onMqttSubscribe event: usually print subscribe acknowledged - optional
// fired by mqttClient.onSubscribe
// useful while debugging subscription errors
void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
  Serial.print(F("Subscribe acknowledged."));
  Serial.print(F("  packetId: "));
  Serial.print(packetId);
  Serial.print(F("  qos: "));
  Serial.println(qos);
}