#ifndef _FFT_DATA_H
#define _FFT_DATA_H

#include <Arduino.h>
#include <APPLICATION/FFT/FFT.h>
#include <APPLICATION\CONSTANTS\dati.h>

extern double Ts;
extern double df;

extern const double ONE_OVER_FFT_SIZE;

//The time in which data was captured. This is equal to FFT_SIZE/sampling_freq
extern const float TOTAL_TIME;

extern float max_magnitude;
extern float fundamental_freq;

// windowing table
extern float window[FFT_SIZE];

// ADC equalization table - see create_equalizer(m)
extern float m[FFT_SIZE >> 1];

// parametri per il calcolo della FFT
extern fft_config_t *real_fft_plan;

// puntatore al buffer dei dati in ingresso per FFT
// viene inizializzato nella funzione process, allo stato
extern IRAM_ATTR float *pInput;

#endif