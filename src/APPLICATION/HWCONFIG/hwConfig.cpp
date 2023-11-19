#include <APPLICATION/HWCONFIG/hwConfig.h>
#include <Arduino.h>
#include <SPI.h>

// canale SPI per connessione con ADC ADS1256 ----------------------------------------
SPIClass hspi = SPIClass(HSPI);

// canale SPI per connessione con ADC MCP3204 ----------------------------------------
SPIClass vspi = SPIClass(VSPI);