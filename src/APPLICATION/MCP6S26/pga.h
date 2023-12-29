#ifndef _PGA_H
#define _PGA_H_H

#include <Arduino.h>

// impostazioni correnti del PGA MCP6S26
typedef struct
{
  uint8_t channel;
  uint8_t gain;
  uint8_t gainValue;
  bool gain_changed;
} stPGA;

extern stPGA pga0;

// comando del guadagno del PGA
// è arrivato un messaggio da pgaSetGainTopic
// deve essere un valore tra 0 e 7; forzato a 0 se esterno all'intervallo o valore non numerico
void setPGAgain(char *data);

#endif