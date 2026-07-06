#include <APPLICATION\MCP3204\mcp3204_data.h>

// MCP3204 ADC output data
mcp3204Data mcp3204_dati;
// MCP3204 sampled data array (4 sequential blocks)
float mcp3204buffer[MCP3204_BUFFER_SIZE];

// queue for ADC access counter of torque sensors
QueueHandle_t xQueueCountADCTorque;
