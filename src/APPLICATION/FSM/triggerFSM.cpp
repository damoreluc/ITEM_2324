#include <APPLICATION/FSM/fsm.h>
#include <APPLICATION\SIM\sim_real_data_selector.h>
#include <APPLICATION/SSD1306/ssd1306.h>

// comando della MSF di acquisizione
// è arrivato un messaggio MQTT da triggerTopic
// ( 0 = Stop, 1 = OneShot, 2 = FreeRun )
void triggerFSM(char *data)
{

  if (strncmp(data, "0", 1) == 0)
  {
    triggered = Stop;
    Serial.println("Fine acquisizione");
    if (getSensMode() == REAL_DATA)
    {
      ssd1306_publish("Stop acquisition\n");
    }
  }

  if (strncmp(data, "1", 1) == 0)
  {
    triggered = OneShot;
    Serial.println("Singola acquisizione");
    if (getSensMode() == REAL_DATA)
    {
      ssd1306_publish("Single shot\n");
    }
  }

  if (strncmp(data, "2", 1) == 0)
  {
    triggered = FreeRun;
    Serial.println("Acquisizione ciclica");
    if (getSensMode() == REAL_DATA)
    {
      ssd1306_publish("Free run\n");
    }
  }
}