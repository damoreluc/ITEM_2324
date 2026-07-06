#ifndef __CUSTOM_H
#define __CUSTOM_H

#include <MQTT/mqtt_functions.h>
// project-specific customizable functions

// loads the list of topics to subscribe to
// Called by configMqttClient()
void topicsToSubscribe();

// loads the list of topics used for publishing
// Called by configMqttClient()
void topicsToPublish();

// operations to execute when a message is received
// called by mqtt_onMqttMessage()
void parseMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

#endif