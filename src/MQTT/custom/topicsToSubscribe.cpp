#include <MQTT/custom/custom.h>
#include <MQTT/custom/mqtt_topics.h>

void topicsToSubscribe()
{
  // // build the subscribed topics dictionary
  // compileSubTopics(subscribedTopics);

  // // number of items in dictionary
  // uint16_t length = subscribedTopics.length();

  // define topics to be subscribed to
  bool result;

  // loop over each dictionary item
  for (uint16_t i = 0; i < maxSubTopic; i++)
  {
    result = AddSubscribedTopic(subTopics[i], 0);
    if (result == false)
    {
      Serial.printf("ERROR: Unable to add topic %s to the list\n", subTopics[i]);
      while (1)
      {
        yield();
      }
    }
  }

  
}