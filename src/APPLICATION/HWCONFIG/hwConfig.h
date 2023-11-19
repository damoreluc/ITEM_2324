#ifndef __HWCONFIG_H
#define __HWCONFIG_H

#include <Arduino.h>
#include <SPI.h>

//#include <APPLICATION\MCP6S26\MCP6S26.h>

/******************************************************************************
 * project specific hardware definition
 *
 *****************************************************************************/

// we drive a led with messages received from ledTopic
// Note: GPIO2 on ESP32 DevKit
#define LED_BUILTIN GPIO_NUM_2

// pin di selezione hardware della modalità di generazione dei dati (simulati o dai sensori fisici)
#define SENS_MODE GPIO_NUM_32

// ingresso digitale ausiliario, optoisolato
#define AUX_DIN GPIO_NUM_33

/*
 *  Mappa interfacciamento con scheda ADS1256
 *    ADS1256           ESP32
 *    +5V               +5V
 *    GND               GND
 *    SCLK     <--      HSPI_SCK (IO14)
 *    DIN      <--      HSPI_MOSI (IO13)
 *    DOUT     -->      HSPI_MISO (IO12)
 *    !DRDY    -->      IO25
 *    !CS      <--      HSPI_CS (IO15)
 *    !PDWN    <--      IO26
 */

#define nDRDY GPIO_NUM_25
#define nCS GPIO_NUM_15
#define nPDWN GPIO_NUM_26

/*
 *  Mappa intefacciamento PGA MCP6S26
 *    MCP6S26          ESP32
 *    VDD               +5V
 *    GND               GND
 *    SCLK     <--      VSPI_SCK (IO18)
 *    SI       <--      VSPI_MOSI (IO23)
 *    SO       -->      not connected
 *    !CS      <--      !CS_PGA0 (IO16)
 */
// MCP6S26 Chip select
#define CS_PGA0 GPIO_NUM_16

/*
 *  Mappa interfacciamento con scheda MAX31865 per RTD1
 *    MAX31865          ESP32
 *    VIN               +3.3V
 *    GND               GND
 *    CLK      <--      VSPI_SCK (IO18)
 *    SDI      <--      VSPI_MOSI (IO23)
 *    SDO      -->      VSPI_MISO (IO19)
 *    !CS      <--      !CS1_MAX31865 (IO17)
 */
// MAX31865 Chip select
#define CS1_MAX31865 GPIO_NUM_17

/*
 *  Mappa interfacciamento con scheda MAX31865 per RTD2
 *    MAX31865          ESP32
 *    VIN               +3.3V
 *    GND               GND
 *    CLK      <--      VSPI_SCK (IO18)
 *    SDI      <--      VSPI_MOSI (IO23)
 *    SDO      -->      VSPI_MISO (IO19)
 *    !CS      <--      !CS2_MAX31865 (IO27)
 */
// MAX31865 Chip select
#define CS2_MAX31865 GPIO_NUM_27

/*
 *  Mappa interfacciamento con ADC MCP3204 per coppie e velocità
 *    MCP3204           ESP32
 *    VIN               +3.3V
 *    GND               GND
 *    CLK      <--      VSPI_SCK (IO18)
 *    SDI      <--      VSPI_MOSI (IO23)
 *    SDO      -->      VSPI_MISO (IO19)
 *    !CS      <--      !CS_MCP3204 (IO5)
 */
// MCP3204 ADC chip select
#define CS_MCP3204 GPIO_NUM_5
// MCP3204 canale coppia 1
#define MCP3204_Torque1 0
// MCP3204 canale coppia 2
#define MCP3204_Torque2 1
// MCP3204 canale velocità 1
#define MCP3204_Speed1 2
// MCP3204 canale velocità 2
#define MCP3204_Speed2 3
// MCP3204 analog Vref
#define MCP3204_VREF 2.50

// // indicatore stato connessione WiFi
// #define pinWiFiConnected GPIO_NUM_23

// canale SPI per connessione con ADC ADS1256 ----------------------------------------
extern SPIClass hspi;

// canale SPI per connessione con ADC MCP3204/RTD/PGA --------------------------------
extern SPIClass vspi;

// -----------------------------------------------------------------------------------

#endif