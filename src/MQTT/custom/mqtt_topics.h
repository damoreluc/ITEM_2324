#ifndef _MQTT_TOPICS_H
#define _MQTT_TOPICS_H

#include <Arduino.h>
#include <dependencies/Dictionary/Dictionary.h>
#include <MQTT/custom/clientID.h>

//Dizionario dei subscribed topics (topic in ingresso alla ESP32)
extern Dictionary<String, String> subscribedTopics;

//Dizionario dei published topics (topic in uscita dalla ESP32)
extern Dictionary<String, String> publishedTopics;

// compila il dizionario dei subscribed topics (da personalizzare)
void compileSubTopics(Dictionary<String, String> &subTopics);

// compila il dizionario dei published topics (da personalizzare)
void compilePubTopics(Dictionary<String, String> &pubTopics);


#endif