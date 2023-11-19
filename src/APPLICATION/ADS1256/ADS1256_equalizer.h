#ifndef _ADS1256_EQUALIZER_H
#define _ADS1256_EQUALIZER_H

#include <Arduino.h>
#include <APPLICATION\CONSTANTS\dati.h>

// accelerometer signal path equalization table - see create_equalizer(m)
extern float m[FFT_SIZE >> 1];

void create_equalizer(float *table);

#endif