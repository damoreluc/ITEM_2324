#ifndef _MQTT_FUNCTIONS_H
#define _MQTT_FUNCTIONS_H 

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <MQTT/custom/mqtt_topics.h>

/******************************************************************************
 * bunch of intermediata functions to ease MQTT support
 * 
 *****************************************************************************/

// Manage list of subscribed topics
// topic to subscribe
const uint8_t MAX_TOPIC_NAME = 100;

typedef struct tSubTopic
{
  char topic[MAX_TOPIC_NAME + 1];
  uint8_t qos;
  uint16_t pktId;
  tSubTopic *next;
} tSubTopic;

// the MQTT client --------------------------------------------------------------------------------
extern AsyncMqttClient mqttClient;

// MQTT helper functions --------------------------------------------------------------------------

// FreeRTOS timer to force MQTT reconnection when WiFi is connected
extern TimerHandle_t mqttReconnectTimer;

// configure mqttclient
void configMqttClient(const char *mqttServer, const int mqttPort, const char *mqttUser, const char *mqttPassword);

// add a new topic to the subscribed list (return TRUE on success)
bool AddSubscribedTopic(const char* topic, uint8_t qos);

// set RTOS timers to handle automatic reconnection to MQTT broker
void setTimersRTOS(uint16_t timeout_ms);

// start RTOS timer to handle automatic reconnection to MQTT broker
void startTimersRTOS();

// ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
void stopTimersRTOS();

// MQTT event handlers ----------------------------------------------------------------------------
// begin connection to MQTT broker
void connectToMqtt();
// handle onMqttConnect event: used to call subscribe topic- mandatory
void onMqttConnect(bool sessionPresent);
// handle onMqttDisconnect event- mandatory
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
// handle onMqttSubscribe event: usually print subscribe acknowledged - optional
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
// handle onMqttUnsubscribe event: usually print unsubscribe acknowledged - optional
void onMqttUnsubscribe(uint16_t packetId);
// handle onMqttMessage event: fired on receiving new message from a subscribed topic - mandatory
// to be implemented on user's code (eg. in main.cpp file)
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
// handle onMqttPublish event: fired when message publish is acknowledged - optional
//   note: publish your message calling mqttClient.publish method
void onMqttPublish(uint16_t packetId);
// print received MQTT message, useful in debugging
void printRcvMsg(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

#endif