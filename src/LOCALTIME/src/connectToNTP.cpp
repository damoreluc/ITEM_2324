#include <LOCALTIME/localTime.h>

// connect to NTP server
void connectToNTP()
{
    // init and get the local time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
