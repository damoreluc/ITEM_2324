#include <MQTT/custom/mqtt_topics.h>

//Dizionario dei subscribed topics (topic in ingresso alla ESP32)
Dictionary<String, String> subscribedTopics = Dictionary<String, String>();

//Dizionario dei published topics (topic in uscita dalla ESP32)
Dictionary<String, String> publishedTopics = Dictionary<String, String>();

// compila il dizionario dei subscribed topics (da personalizzare)
void compileSubTopics(Dictionary<String, String> &subTopics) {
  // subscribed topic di comando del led giallo  
  subTopics.set("yellowOnOffTopic", thisClient "/yellowTopic");
  // subscribed topic di comando del led rosso
  subTopics.set("redOnOffTopic", thisClient "/redTopic");
  // subscribed topic di comando del led blu
  subTopics.set("blueOnOffTopic", thisClient "/blueTopic");
  // subscribed topic per stampa messaggi
  subTopics.set("inputTopic", thisClient "/input");
}

// compila il dizionario dei published topics (da personalizzare)
void compilePubTopics(Dictionary<String, String> &pubTopics) {
  // topic di pubblicazione messaggi
  pubTopics.set("outTopic", thisClient "/output");
}