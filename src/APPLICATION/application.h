#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <Arduino.h>

// comanda on/off led rosso a partire dal payload
void driveOnOffRed(char* data);
// comanda on/off led giallo a partire dal payload
void driveOnOffYellow(char* data);
// comanda on/off led blu a partire dal payload
void driveOnOffBlue(char* data);

#endif