#include <APPLICATION\boot.h>
#include <Arduino.h>
#include <MQTT\custom\mqtt_topics.h>

void bootMsg(const char *mqttServer, char subTopics[][50], char pubTopics[][50])
{
    // some debug/informative message
    Serial.println();
    Serial.println(F("ESP32-WROOM-32 DevKit ADS1256 ADC 24 bit, FFT, RTD with MAX31865, Async MQTT client v2.0, binary data"));
    Serial.print(F(" MQTT broker: "));
    Serial.println(mqttServer);
    Serial.println(F(" Subscribed topics:"));
    for (int i = 0; i < maxSubTopic; i++)
    {
        Serial.printf("\t%s\n", subTopics[i]);
    }

    Serial.println(F("\n Published topics:"));
    for (int i = 0; i < maxPubTopic; i++)
    {
        Serial.printf("\t%s\n", pubTopics[i]);
    }

    Serial.println(F("Tries automatic reconnection to MQTT in case of network errors."));
    Serial.println(F("Note: you can try MQTTLens to check MQTT functionalities"));
    Serial.println();
}