#include <MQTT/mqtt_functions.h>

// handle onMqttUnsubscribe event: usually print unsubscribe acknowledged - optional
// fired by mqttClient.onUnsubscribe
void onMqttUnsubscribe(uint16_t packetId)
{
  Serial.println(F("Unsubscribe acknowledged."));
  Serial.print(F("  packetId: "));
  Serial.println(packetId);
}
