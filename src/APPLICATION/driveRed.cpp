#include <APPLICATION/application.h>
#include <HWCONFIG/hwConfig.h>

// comanda on/off led rosso a partire dal payload
void driveOnOffRed(char *data)
{
    if (strncmp(data, "0", 1) == 0)
    {
        digitalWrite(pinRed, LOW);
        Serial.println("led rosso spento");
    }
    else if (strncmp(data, "1", 1) == 0)
    {
        digitalWrite(pinRed, HIGH);
        Serial.println("led rosso acceso");
    }
}