#include <MQTT/mqtt_functions.h>

extern tSubTopic *pListOfTopics;

// handle onMqttConnect event: used to call subscribe topic- mandatory
// fired by mqttClient.onConnect
// Note: place here all the subscribing you need, use mqttClient.subscribe()
void onMqttConnect(bool sessionPresent)
{
  Serial.print(F("Connected to MQTT. Client ID: "));
  Serial.println(mqttClient.getClientId());

  // set RTOS timers to handle automatic reconnection to MQTT broker
  setTimersRTOS(4000);

  // traverse the list of topics to subscribe to
  tSubTopic *ptr = pListOfTopics;
  while (ptr != NULL)
  {
    ptr->pktId = mqttClient.subscribe(ptr->topic, ptr->qos);
    Serial.printf("Subscribing %s at QoS %d, packetId: %d\n", ptr->topic, ptr->qos, ptr->pktId);

    // point to the next item
    ptr = ptr->next;
  }

}