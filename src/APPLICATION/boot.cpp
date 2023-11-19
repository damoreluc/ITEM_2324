#include <APPLICATION\boot.h>
#include <Arduino.h>

void bootMsg(const char *mqttServer, Dictionary<String, String> &subTopics, Dictionary<String, String> &pubTopics)
{
    // some debug/informative message
    Serial.println();
    Serial.println(F("ESP32-WROOM-32 DevKit ADS1256 ADC 24 bit, FFT, RTD with MAX31865, Async MQTT client v2.0, binary data"));
    Serial.print(F(" MQTT broker: "));
    Serial.println(mqttServer);
    Serial.println(F(" Subscribed topics:"));
    for (int i = 0; i < subTopics.length(); i++)
    {
        Serial.printf("\t%s\n", subTopics.get(i).c_str());
    }

    
    Serial.println(F("\n Published topics:"));
    for (int i = 0; i < pubTopics.length(); i++)
    {
        Serial.printf("\t%s\n", pubTopics.get(i).c_str());
    }

    Serial.println(F("Tries automatic reconnection to MQTT in case of network errors."));
    Serial.println(F("Note: you can try MQTTLens to check MQTT functionalities"));
    Serial.println();
}