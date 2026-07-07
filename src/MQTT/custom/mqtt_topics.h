#ifndef _MQTT_TOPICS_H
#define _MQTT_TOPICS_H

#include <Arduino.h>
#include <dependencies/Dictionary/Dictionary.h>

// Dictionary of subscribed topics (incoming topics for ESP32)
extern Dictionary<String, String> subscribedTopics;

// Dictionary of published topics (outgoing topics from ESP32)
extern Dictionary<String, String> publishedTopics;

// prefisso runtime dei topic MQTT (default definito in MQTT/ClientID.h)
void setTopicPrefix(const char *topicPrefix);

// builds the subscribed topics dictionary (customizable)
void compileSubTopics(Dictionary<String, String> &subTopics);

// builds the published topics dictionary (customizable)
void compilePubTopics(Dictionary<String, String> &pubTopics);


#endif