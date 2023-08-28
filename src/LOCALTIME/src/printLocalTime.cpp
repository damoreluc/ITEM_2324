#include <LOCALTIME/localTime.h>

// pretty print local time and date
void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println(F("Failed to obtain time"));
    return;
  }
  Serial.println(&timeinfo, "%A %d %B %Y %H:%M:%S");
}
