#ifndef _ISR_H
#define _ISR_H

#include <Arduino.h>
#include <APPLICATION\CONSTANTS\dati.h>

// ✅ FIX #6: Spinlock for countData synchronization between ISR and FSM
extern portMUX_TYPE countDataMux;

// ISR that handles the arrival of a new sample on DRDY falling edge
void IRAM_ATTR ISR_DRDY();

#endif