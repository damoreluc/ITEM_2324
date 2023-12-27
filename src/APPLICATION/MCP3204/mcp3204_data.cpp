#include <APPLICATION\MCP3204\mcp3204_data.h>

// MCP3204 ADC output data
mcp3204Data mcp3204_dati;
// array dei campionamenti con MCP3204 (4 blocchi sequenziali)
float mcp3204buffer[MCP3204_BUFFER_SIZE];

// coda per conteggio accessi ADC dei sensori di coppia
QueueHandle_t xQueueCountADCTorque;
