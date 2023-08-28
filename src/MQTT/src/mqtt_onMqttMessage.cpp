#include <MQTT/mqtt_functions.h>
#include <MQTT/custom/custom.h>

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    // defer message parsing to user procedure
    parseMessage(topic, payload, properties, len, index, total);
}
