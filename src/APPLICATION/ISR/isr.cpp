#include <APPLICATION\ISR\isr.h>
#include <APPLICATION\ADS1256\ADS1256Ext.h>
#include <APPLICATION\HWCONFIG\hwConfig.h>

// ISR for the management of the arrival of a new sample on the DRDY falling edge
// uses the xQueueADS1256Sample queue to tell the fsm the index of the sample to be scanned

// provare ad usare taskNotify per sincronizzare l'acquisizione tra questa ISR e la FSM nello stato Sampling
void IRAM_ATTR ISR_DRDY()
{
  BaseType_t xHigherPriorityTaskWoken;

  if (countData < FFT_SIZE)
  {
    // get new sample from ADS1256
    // the ReadRawData() method raises unpredictable glitches on RTOS execution
    // that lead to system reboot.
    // work around: move the call into the fsm

    // push new data into ADS1256 queue
    xQueueSendFromISR(xQueueADS1256Sample, (void *)&countData, &xHigherPriorityTaskWoken);
    countData++;

    // Now we can switch context if necessary
    if (xHigherPriorityTaskWoken)
    {
      /* Actual macro used here is port specific. */
      portYIELD_FROM_ISR();
    }
  }
  else
  {
    detachInterrupt(nDRDY);
    countData = 0;
  }
}