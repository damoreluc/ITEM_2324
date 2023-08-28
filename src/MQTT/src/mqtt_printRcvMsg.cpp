#include <MQTT/mqtt_functions.h>
#include <LOCALTIME/localTime.h>

  // print some information about the received message
void printRcvMsg(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  Serial.print(F("Publish received on "));
  printLocalTime();
  Serial.print(F("  topic: "));
  Serial.print(topic);
  Serial.print(F("  payload: "));
  for (int i = 0; i < len; i++)
  {
    Serial.print(payload[i]);
  }
  Serial.print("  qos: ");
  Serial.print(properties.qos);
  Serial.print(F("  dup: "));
  Serial.print(properties.dup);
  Serial.print(F("  retain: "));
  Serial.print(properties.retain);
  Serial.print(F("  len: "));
  Serial.print(len);
  Serial.print(F("  index: "));
  Serial.print(index);
  Serial.print(F("  total: "));
  Serial.println(total);
  Serial.println();
}