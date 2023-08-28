#ifndef _LOCALTIME_H
#define _LOCALTIME_H

#include <Arduino.h>

// NTP local time synchronization -----------------------------------------------------------------
// https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/
extern const char *ntpServer;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;

// connect to NTP server
void connectToNTP();

// pretty print local time and date
void printLocalTime();

#endif