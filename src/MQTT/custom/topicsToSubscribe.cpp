#include <MQTT/custom/custom.h>
#include <MQTT/custom/mqtt_topics.h>

void topicsToSubscribe()
{
  // build the subscribed topics dictionary
  compileSubTopics(subscribedTopics);

  // number of items in dictionary
  uint16_t length = subscribedTopics.length();

  // define topics to be subscribed to
  bool result;

  // loop over each dictionary item
  for (uint16_t i = 0; i < length; i++)
  {
    result = AddSubscribedTopic(subscribedTopics.get(i).c_str(), 0);
    if (result == false)
    {
      Serial.printf("ERROR: Unable to add topic %s to the list\n", subscribedTopics.getKey(i).c_str());
      while (1)
      {
        yield();
      }
    }
  }

  
}