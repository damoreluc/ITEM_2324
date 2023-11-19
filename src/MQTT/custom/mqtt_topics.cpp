#include <MQTT/custom/mqtt_topics.h>

//Dizionario dei subscribed topics (topic in ingresso alla ESP32)
Dictionary<String, String> subscribedTopics = Dictionary<String, String>();

//Dizionario dei published topics (topic in uscita dalla ESP32)
Dictionary<String, String> publishedTopics = Dictionary<String, String>();

// compila il dizionario dei subscribed topics (da personalizzare)
void compileSubTopics(Dictionary<String, String> &subTopics) {
  // subscribed topic di comando del guadagno del PGA
  subTopics.set("pgaSetGainTopic", thisClient "/pgaSetGain");
  // subscried topic di comando della MSF (one-shot, free-run, stop)
  subTopics.set("triggerTopic", thisClient "/trigger");  
  // subscribed topic per stampa messaggi
  subTopics.set("inTopic", thisClient "/inTopic");
}

// compila il dizionario dei published topics (da personalizzare)
void compilePubTopics(Dictionary<String, String> &pubTopics) {
  // topic di pubblicazione spettro accelerometro 0
  pubTopics.set("outTopic0", thisClient "/FFTBinTopic0");
  // topic di pubblicazione spettro accelerometro 1
  pubTopics.set("outTopic1", thisClient "/FFTBinTopic1");
  // topic di pubblicazione valore RTD1
  pubTopics.set("outTopic2", thisClient "/RTD1BinTopic"); 
  // topic di pubblicazione stato RTD1
  pubTopics.set("outTopic3", thisClient "/RTD1FaultTopic"); 
  // topic di pubblicazione valore RTD2
  pubTopics.set("outTopic4", thisClient "/RTD2BinTopic"); 
  // topic di pubblicazione stato RTD2
  pubTopics.set("outTopic5", thisClient "/RTD2FaultTopic"); 
  // topic di pubblicazione conteggio campioni ADC coppia/velocità
  pubTopics.set("outTopic6", thisClient "/CountAdcTopic");
  // topic di pubblicazione valori coppia/velocità
  pubTopics.set("outTopic7", thisClient "/TorqueSpeedTopic");
  // topic di pubblicazione valore attuale del guadagno PGA
  pubTopics.set("pgaGetGainTopic", thisClient "/pgaGetGain");
  }