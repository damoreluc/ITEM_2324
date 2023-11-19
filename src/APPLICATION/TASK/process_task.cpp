#include <APPLICATION/TASK/task.h>
#include <APPLICATION/FSM/fsm.h>

// handle del task di acquisizione dati ed elaborazione FFT
TaskHandle_t processTaskHandle;

// index of PGA MCP6S26 current channel
uint8_t MCP6S26_current_channel_index = 0;
// index of PGA MCP6S26 channel to publish
uint8_t MCP6S26_publish_channel_index = 0;

// process task ------------------------------------------------------------------------
void process(void *pvParameters) {

  while (1)
  {
    fsm();
  }  
}