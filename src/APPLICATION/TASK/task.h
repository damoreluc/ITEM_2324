#ifndef _TASK_H
#define _TASK_H

#include <Arduino.h>

// indice del canale accelerometrico da acquisire
// index of PGA MCP6S26 current channel
extern uint8_t MCP6S26_current_channel_index;

// indice del canale accelerometrico da pubblicare
// index of PGA MCP6S26 channel to publish
extern uint8_t MCP6S26_publish_channel_index;

// handle dei task usati dalla MSF
// handle del task di elaborazione FFT e stampa
extern TaskHandle_t processTaskHandle;
// handle del task di pubblicazione FFT
extern TaskHandle_t publishTaskHandle;
// handle del task di acquisizione con MCP3204
extern TaskHandle_t sampleMCP3204TaskHandle;

// process task ------------------------------------------------------------------------
void process(void *pvParameters);

// publish task ------------------------------------------------------------------------
void publishFFT(void *pvParameters);

// task acquisizione con MCP3204 -------------------------------------------------------
// task gestione ADC MCP3204
void sampleMCP3204(void *pvParameters);

#endif