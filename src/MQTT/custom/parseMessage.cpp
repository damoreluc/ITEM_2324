#include <MQTT/mqtt_functions.h>
#include <MQTT/custom/custom.h>
#include <MQTT/custom/mqtt_topics.h>
#include <APPLICATION\HWCONFIG\hwConfig.h>
#include <APPLICATION\FSM\fsm.h>
#include <APPLICATION\MCP6S26\pga.h>
#include <APPLICATION/application.h>

// What to do when a message is received
// is invoked by mqtt_onMqttMessage()
void parseMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    // payload clean up
    // In a dynamic buffer, it extracts only the first len characters of the payload
    char *data = (char *)malloc((len + 1) * sizeof(char));
    strncpy(data, payload, len);
    // data is a C string: you have to end it with the terminator '\0'
    data[len] = '\0';

    // print some information about the received message
    printRcvMsg(topic, payload, properties, len, index, total);

    // to be customized...

    // MSF Acquisition Command
    // A message arrived from triggerTopic
    // ( 0 = Stop, 1 = OneShot, 2 = FreeRun )
    if (strcmp(topic, subscribedTopics.get("triggerTopic").c_str()) == 0)
    {
        // commands the MSF of data acquisition management
        triggerFSM(data);
    }

    // PGA Gain Control
    // A message arrived from pgaSetGainTopic
    // it must be a value between 0 and 7; forced to 0 if out of range or non-numeric value
    else if (strcmp(topic, subscribedTopics.get("pgaSetGainTopic").c_str()) == 0) 
    {
        // modifies the gain of the PGA MCP6S26
        setPGAgain(data);
    }  

    // Release the dynamic buffer
    free(data);
}