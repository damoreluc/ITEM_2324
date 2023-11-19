#ifndef _BOOT_H
#define _BOOT_H

#include <dependencies/Dictionary/Dictionary.h>
#include <Arduino.h>
// print some boot messages on serial monitor
void bootMsg(const char *mqttServer, Dictionary<String, String> &subTopics, Dictionary<String, String> &pubTopics);

#endif