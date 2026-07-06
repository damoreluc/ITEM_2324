#ifndef _SIM_REAL_DATA_SEL
#define _SIM_REAL_DATA_SEL

#include <Arduino.h>
#include <APPLICATION\HWCONFIG\hwConfig.h>

// data generation mode
typedef enum {REAL_DATA, SYM_DATA} eSensMode;

// reads SENS_MODE pin and updates simulated_data/real_data state
void readSensMode();

// returns simulated_data/real_data state
eSensMode getSensMode();

#endif