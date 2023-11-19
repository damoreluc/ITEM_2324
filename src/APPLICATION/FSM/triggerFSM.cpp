#include <APPLICATION/FSM/fsm.h>

// comando della MSF di acquisizione
// è arrivato un messaggio MQTT da triggerTopic
// ( 0 = Stop, 1 = OneShot, 2 = FreeRun )
void triggerFSM(char *data) {
    
if (strncmp(data, "0", 1) == 0)
    {
      triggered = Stop;
      Serial.println("Fine acquisizione");
    }

    if (strncmp(data, "1", 1) == 0)
    {
      triggered = OneShot;
      Serial.println("Singola acquisizione");
    }

    if (strncmp(data, "2", 1) == 0)
    {
      triggered = FreeRun;
      Serial.println("Acquisizione ciclica");
    }
}