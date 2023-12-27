#include <APPLICATION/TASK/task.h>
#include <APPLICATION\MCP3204\mcp3204.h>
#include <APPLICATION\MCP3204\mcp3204_data.h>
#include <APPLICATION\HWCONFIG\hwConfig.h>

// handle del task di acquisizione con MCP3204
TaskHandle_t sampleMCP3204TaskHandle;

// speed/torque acquisition task ------------------------------------------------------------------------
void sampleMCP3204(void *pvParameters)
{
  while (1)
  {
    // hang here until FFT is computed
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (mcp3204_BufferAvailable() > 0)
    {

      mcp3204_getAllVoltage(vspi, CS_MCP3204, &mcp3204_dati);

      /*
       * debug
       
      mcp3204_dati.volt0 = 2.5;
      mcp3204_dati.volt1 = 3.0;
      mcp3204_dati.volt2 = 1.5;
      mcp3204_dati.volt3 = 2.0;
      
       * fine debug
       */
      mcp3204_Push(&mcp3204_dati);
    }
  }
}