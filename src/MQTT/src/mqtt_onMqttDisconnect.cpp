#include <MQTT/mqtt_functions.h>

// fired by mqttClient.onDisconnect and debug printing- mandatory
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{

  Serial.print(F("Disconnected from MQTT. Reason: "));
  switch (reason)
  {
  case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED:
    Serial.println(F("TCP disconnected"));
    break;
  case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
    Serial.println(F("MQTT unacceptable protocolo version"));
    break;
  case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED:
    Serial.println(F("MQTT identifier rejected"));
    break;
  case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE:
    Serial.println(F("MQTT server unavailable"));
    break;
  case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS:
    Serial.println(F("MQTT malformed credentials"));
    break;
  case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED:
    Serial.println(F("MQTT not authorized"));
    break;
  case AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE:
    Serial.println(F("ESP8266 not enough space"));
    break;
  case AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT:
    Serial.println(F("TLS bad fingerprint"));
    break;
  default:
    Serial.println(F("unknown"));
    break;
  }

  // try reconnect to MQTT broker if WiFi is still ok
  if (WiFi.isConnected())
  {
    startTimersRTOS();
  }
}