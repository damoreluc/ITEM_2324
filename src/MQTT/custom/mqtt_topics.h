#ifndef _MQTT_TOPICS_H
#define _MQTT_TOPICS_H

#include <Arduino.h>
//#include <dependencies/Dictionary/Dictionary.h>
#include <MQTT/custom/clientID.h>

// subscribed topics enumerator
typedef enum {pgaSetGainTopic, triggerTopic, inTopic, maxSubTopic} eSubTopics;

// publish topics enumerator
typedef enum {outTopic0, outTopic1, outTopic2, outTopic3, outTopic4, outTopic5, outTopic6, outTopic7, pgaGetGainTopic, maxPubTopic} ePubTopics;

//Dizionario dei subscribed topics (topic in ingresso alla ESP32)
//extern Dictionary<String, String> subscribedTopics;
extern char subTopics[maxSubTopic][50];

//Dizionario dei published topics (topic in uscita dalla ESP32)
//extern Dictionary<String, String> publishedTopics;
extern char pubTopics[maxPubTopic][50];

// // compila il dizionario dei subscribed topics (da personalizzare)
// void compileSubTopics(Dictionary<String, String> &subTopics);

// // compila il dizionario dei published topics (da personalizzare)
// void compilePubTopics(Dictionary<String, String> &pubTopics);


#endif