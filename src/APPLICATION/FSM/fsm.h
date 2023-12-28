#ifndef _FSM_H
#define _FSM_H

#include <Arduino.h>

// modalità di acquisizione
typedef enum
{
  Stop,
  OneShot,
  FreeRun
} tMode;

extern tMode triggered;

// macchina a stati finiti acquisizione
typedef enum
{
  StartADC,
  Sampling,
  Compute,
  Publish,
  WaitTrigger
} tStati;

// flag true quando i dati sono pronti
extern volatile bool dataReady;

// stato della MSF
extern volatile tStati _stato;

// some additional informations on task execution
extern uint32_t freeHeap;
extern uint32_t elapsedTime;

// MSF di acquisizione
void fsm();

// comando della MSF di acquisizione
// è arrivato un messaggio MQTT da triggerTopic
// ( 0 = Stop, 1 = OneShot, 2 = FreeRun )
void triggerFSM(char *data);

#endif