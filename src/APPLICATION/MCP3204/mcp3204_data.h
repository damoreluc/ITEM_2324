#ifndef _MCP3204_DATA_H
#define _MCP3204_DATA_H

#include <Arduino.h>
#include <APPLICATION\MCP3204\mcp3204.h>
#include <APPLICATION\CONSTANTS\dati.h>

// MCP3204 ADC oputput data
extern mcp3204Data mcp3204_dati;

extern float mcp3204buffer[MCP3204_BUFFER_SIZE];

// coda per conteggio accessi ADC dei sensori di coppia
extern QueueHandle_t xQueueCountADCTorque;

#endif