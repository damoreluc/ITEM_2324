#ifndef _ADS1256EXT_H
#define _ADS1256EXT_H

#include <Arduino.h>
#include <APPLICATION\ADS1256\ADS1256.h>
#include <APPLICATION\CONSTANTS\dati.h>

#define ADS1256QueueSize 20 //FFT_SIZE

// creation of queue for ADS1256 ADC data acquisition from ISR
extern QueueHandle_t xQueueADS1256Sample;

class ADS1256Ext : public ADS1256 {
    public: 
        ADS1256Ext();  // constructor
        void setup();  // initial setup
        typedef enum {PGA_1 = 0, PGA_2 = 1, PGA_4 = 2, PGA_8 = 3, PGA_16 = 4, PGA_32 = 5, PGA_64 = 6} pgaGains;
        void setGain(pgaGains gain);
    private:
};

// ADC instance
extern ADS1256Ext adc;

// ADS1256 DRDY
extern volatile int32_t countData;

#endif