#include <MQTT/mqtt_functions.h>
#include <MQTT/custom/custom.h>

// configure mqttclient
void configMqttClient(const char *mqttServer, const int mqttPort, const char *mqttUser, const char *mqttPassword) {
  // set mqttClient event's callback functions
  mqttClient.onConnect(onMqttConnect);           // mandatory
  mqttClient.onDisconnect(onMqttDisconnect);     // mandatory
// mqttClient.onSubscribe(onMqttSubscribe);      // optional (subscribing to topic acknowledged)
// mqttClient.onUnsubscribe(onMqttUnsubscribe);  // optional
  mqttClient.onMessage(onMqttMessage);           // mandatory
  mqttClient.onPublish(onMqttPublish);           // optional (publishing acknowledged)
  
  // set Client ID
  mqttClient.setClientId(thisClient);
  // set MQTT broker server
  mqttClient.setServer(mqttServer, mqttPort);

  // set user's credentials on MQTT broker
  mqttClient.setCredentials(mqttUser, mqttPassword);

  // add the list of subscribed topics
  topicsToSubscribe();

  // add the list of publishing topics
  topicsToPublish();
}