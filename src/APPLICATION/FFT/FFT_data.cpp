#include <APPLICATION/FFT/FFT_data.h>
//#include <APPLICATION\CONSTANTS\dati.h>

double Ts = 1.0/((float)FSAMPLE);
double df = 1.0 / (Ts * FFT_SIZE);

const double ONE_OVER_FFT_SIZE = 1.0 / ((double)FFT_SIZE);

//The time in which data was captured. This is equal to FFT_SIZE/sampling_freq
const float TOTAL_TIME=(float)FFT_SIZE/(float)FSAMPLE;

// ADC equalization table - see create_equalizer(m)
float m[FFT_SIZE >> 1];
 
//float fft_input[FFT_SIZE];
//float fft_output[FFT_SIZE];

float max_magnitude = 0;
float fundamental_freq = 0;

// parametri per il calcolo della FFT
fft_config_t *real_fft_plan;

// puntatore al buffer dei dati in ingresso per FFT
// viene inizializzato nella funzione process, allo stato
IRAM_ATTR float *pInput;