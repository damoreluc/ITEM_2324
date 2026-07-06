#ifndef _RTD_DATA_H
#define _RTD_DATA_H

#include <Arduino.h>
#include <APPLICATION\MAX31865\rtd_MAX31865.h>

// struct per le temperature delle RTD
typedef struct
{
  float rtd1;
  float rtd2;
  char fault1[MSG_FAULT_LEN + 1];
  char fault2[MSG_FAULT_LEN + 1];
} stRTD;

extern stRTD temperature;

// coda per temperature RTD
extern QueueHandle_t xQueueRTD;

// RTD1 and RTD2 module initialization
void setupRTD();

// lettura RTD1 ed RTD2 e inserimento dati in xQueueRTD ogni RTD_PERIOD ms
void readRTD();

#endif