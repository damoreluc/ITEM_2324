#include <MQTT/custom/custom.h>
#include <MQTT/custom/mqtt_topics.h>

// loads the list of topics used for publishing
// Called by configMqttClient()
void topicsToPublish() {
  // build the publishing topics dictionary
  compilePubTopics(publishedTopics);    
}