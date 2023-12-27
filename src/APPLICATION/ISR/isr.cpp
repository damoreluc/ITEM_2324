#include <APPLICATION\ISR\isr.h>
#include <APPLICATION\ADS1256\ADS1256Ext.h>

// ISR per la gestione dell'arrivo di un nuovo campione sul fronte di discesa di DRDY

void IRAM_ATTR ISR_DRDY()
{
  BaseType_t xHigherPriorityTaskWoken;
  int32_t adcValue;

  if (countData < FFT_SIZE)
  {

    newData = true;

    // get new sample from ADS1256
    adcValue = adc.ReadRawData();

    // push new data into ADS1256 queue
    // xQueueSendFromISR(xQueueADS1256Sample, (void *)&countData, &xHigherPriorityTaskWoken);
    xQueueSendFromISR(xQueueADS1256Sample, (void *)&adcValue, &xHigherPriorityTaskWoken);
    countData++;

    // Now we can switch context if necessary
    if (xHigherPriorityTaskWoken)
    {
      /* Actual macro used here is port specific. */
      portYIELD_FROM_ISR();
    }
  } else {
    countData = 0;
  }
}