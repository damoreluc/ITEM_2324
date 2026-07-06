#include <APPLICATION/HWCONFIG/hwConfig.h>
#include <Arduino.h>
#include <SPI.h>

// SPI channel connected to ADS1256 ADC ----------------------------------------
SPIClass hspi = SPIClass(HSPI);

// SPI channel connected to MCP3204 ADC ----------------------------------------
SPIClass vspi = SPIClass(VSPI);