#include <MQTT/mqtt_functions.h>
#include <MQTT/custom/custom.h>
#include <MQTT/custom/mqtt_topics.h>
#include <HWCONFIG/hwConfig.h>
#include <APPLICATION/application.h>

// operazioni da eseguire quando viene ricevuto un messaggio
// viene richiamata da mqtt_onMqttMessage()
void parseMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    // bonifica del payload
    // estrae solo i primi len caratteri del payload
    char data[len + 1];
    strncpy(data, payload, len);

    // print some information about the received message
    printRcvMsg(topic, payload, properties, len, index, total);

    // da personalizzare

    // comando del led giallo
    // è arrivato un messaggio da yellowOnOffTopic
    if (strcmp(topic, subscribedTopics.get("yellowOnOffTopic").c_str()) == 0)
    {
        // comanda on/off led giallo a partire dal payload
        driveOnOffYellow(data);
    }

    // comando del led rosso
    // è arrivato un messaggio da redOnOffTopic
    else if (strcmp(topic, subscribedTopics.get("redOnOffTopic").c_str()) == 0) 
    {
        // comanda on/off led rosso a partire dal payload
        driveOnOffRed(data);
    }

    // comando del led blu
    // è arrivato un messaggio da blueOnOffTopic
    else if (strcmp(topic, subscribedTopics.get("blueOnOffTopic").c_str()) == 0) 
    {
        // comanda on/off led blu a partire dal payload
        driveOnOffBlue(data);
    }    
}