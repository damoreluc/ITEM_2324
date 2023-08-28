#ifndef __CUSTOM_H
#define __CUSTOM_H

#include <MQTT/mqtt_functions.h>
// funzioni da personalizzare in base al progetto

// carica la lista dei topics che si vogliono sottoscrivere
// Viene richiamata nella configMqttClient()
void topicsToSubscribe();

// carica la lista dei topics su cui si andrà a pubblicare
// Viene richiamata nella configMqttClient()
void topicsToPublish();

// operazioni da eseguire quando viene ricevuto un messaggio
// viene richiamata da mqtt_onMqttMessage()
void parseMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

#endif