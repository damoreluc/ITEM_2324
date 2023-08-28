#include <APPLICATION/application.h>
#include <HWCONFIG/hwConfig.h>

// comanda on/off led giallo a partire dal payload
void driveOnOffYellow(char *data)
{
    if (strncmp(data, "0", 1) == 0)
    {
        digitalWrite(pinYellow, LOW);
        Serial.println("led giallo spento");
    }
    else if (strncmp(data, "1", 1) == 0)
    {
        digitalWrite(pinYellow, HIGH);
        Serial.println("led giallo acceso");
    }
}