#ifndef _ISR_H
#define _ISR_H

#include <Arduino.h>
#include <APPLICATION\CONSTANTS\dati.h>

// ISR per la gestione dell'arrivo di un nuovo campione sul fronte di discesa di DRDY
void IRAM_ATTR ISR_DRDY();

#endif