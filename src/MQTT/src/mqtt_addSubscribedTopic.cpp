#include <MQTT/mqtt_functions.h>

// Manage list of subscribed topics
// topic to subscribe
tSubTopic *pListOfTopics = NULL;

// add a new topic to the subscribed list (return TRUE on success)
bool AddSubscribedTopic(const char *topic, uint8_t qos)
{

  tSubTopic *item = (tSubTopic *)malloc(sizeof(tSubTopic));
  if (item == NULL)
  {
    return false;
  }
  else
  {
    strncpy(item->topic, topic, MAX_TOPIC_NAME);
    item->qos = qos;
    item->pktId = 0;

    // point to the old first one
    item->next = pListOfTopics;

    // point first to the new first node
    pListOfTopics = item;

    return true;
  }
}