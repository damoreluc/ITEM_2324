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
// a message was received from pgaSetGainTopic
// must be a value between 0 and 7; forced to 0 if out of range or non-numeric
void setPGAgain(char *data);

#endif