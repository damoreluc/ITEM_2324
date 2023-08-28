#include <MQTT/custom/custom.h>
#include <MQTT/custom/mqtt_topics.h>

// carica la lista dei topics su cui si andrà a pubblicare
// Viene richiamata nella configMqttClient()
void topicsToPublish() {
  // build the publishing topics dictionary
  compilePubTopics(publishedTopics);    
}