#include <APPLICATION/MAX31865/rtd_data.h>
#include <APPLICATION\CONSTANTS\dati.h>
#include <APPLICATION\SIM\sim_real_data_selector.h>

// struct per le temperature delle RTD
stRTD temperature;

// coda per temperature RTD
QueueHandle_t xQueueRTD;

// RTD1 module instance:
// use hardware SPI, just pass in the CS pin
// spi module: VSPI (MISO = GPIO19, MOSI = GPIO23, SCK = GPIO18)
Adafruit_MAX31865 RTD1 = Adafruit_MAX31865(CS1_MAX31865);

// RTD2 module instance:
// use hardware SPI, just pass in the CS pin
// spi module: VSPI (MISO = GPIO19, MOSI = GPIO23, SCK = GPIO18)
Adafruit_MAX31865 RTD2 = Adafruit_MAX31865(CS2_MAX31865);

// inizializzazione moduli RDT1 e RTD2
void setupRTD()
{
  // setup RTD1 object: set to 2WIRE, 3WIRE or 4WIRE as necessary
  RTD1.begin(MAX31865_3WIRE);
  delay(250);
  // setup RTD2 object: set to 2WIRE, 3WIRE or 4WIRE as necessary
  RTD2.begin(MAX31865_3WIRE);
  delay(250);
}

// lettura RTD1 ed RTD2 e inserimento dati in xQueueRTD ogni RTD_PERIOD ms
void readRTD()
{
  static long prev = millis();
  long now = millis();

  stRTD temp;

  if (now - prev >= RTD_PERIOD)
  {
    prev = now;
    // lettura del sensore RTD1
    if (getSensMode() == REAL_DATA)
    {
      getTemperature(RTD1, &temp.rtd1, &temp.fault1[0]);
    }

    if (getSensMode() == SYM_DATA)
    {
      temp.rtd1 = 20.0;
      strcpy(&temp.fault1[0], "Ok");
    }

    // lettura del sensore RTD2
    if (getSensMode() == REAL_DATA)
    {
      getTemperature(RTD2, &temp.rtd2, &temp.fault2[0]);
    }

    if (getSensMode() == SYM_DATA)
    {
      temp.rtd2 = temp.rtd1 + 0.5;
      strcpy(&temp.fault2[0], "Ok");
    }

    // inserimento in coda
    if (uxQueueSpacesAvailable(xQueueRTD) > 0)
    {
      xQueueSendToBack(xQueueRTD, (void *)&temp, portMAX_DELAY);
    }
  }
}