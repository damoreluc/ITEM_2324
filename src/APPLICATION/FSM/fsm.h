#ifndef _FSM_H
#define _FSM_H

#include <Arduino.h>

// acquisition mode
typedef enum
{
  Stop,
  OneShot,
  FreeRun
} tMode;

extern tMode triggered;

// acquisition finite-state machine
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

// MSF state
extern volatile tStati _stato;

// some additional informations on task execution
extern uint32_t freeHeap;
extern uint32_t elapsedTime;

// acquisition FSM
void fsm();

// acquisition FSM command
// a MQTT message was received from triggerTopic
// ( 0 = Stop, 1 = OneShot, 2 = FreeRun )
void triggerFSM(char *data);

#endif