#ifndef _TASK_H
#define _TASK_H

#include <Arduino.h>

// index of accelerometer channel to acquire
// index of PGA MCP6S26 current channel
extern uint8_t MCP6S26_current_channel_index;

// index of accelerometer channel to publish
// index of PGA MCP6S26 channel to publish
extern uint8_t MCP6S26_publish_channel_index;

// handle dei task usati dalla MSF
// FFT processing and print task handle
extern TaskHandle_t processTaskHandle;
// FFT publishing task handle
extern TaskHandle_t publishTaskHandle;
// MCP3204 acquisition task handle
extern TaskHandle_t sampleMCP3204TaskHandle;

// process task ------------------------------------------------------------------------
void process(void *pvParameters);

// publish task ------------------------------------------------------------------------
void publishFFT(void *pvParameters);

// MCP3204 acquisition task -------------------------------------------------------
// MCP3204 ADC management task
void sampleMCP3204(void *pvParameters);

#endif