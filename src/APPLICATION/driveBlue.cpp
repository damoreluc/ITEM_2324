#include <APPLICATION/application.h>
#include <HWCONFIG/hwConfig.h>

// comanda on/off led blu a partire dal payload
void driveOnOffBlue(char *data)
{
    if (strncmp(data, "0", 1) == 0)
    {
        digitalWrite(pinBlue, LOW);
        Serial.println("led blu spento");
    }
    else if (strncmp(data, "1", 1) == 0)
    {
        digitalWrite(pinBlue, HIGH);
        Serial.println("led blu acceso");
    }
}