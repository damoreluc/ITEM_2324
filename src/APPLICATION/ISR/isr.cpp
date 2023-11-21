#include <APPLICATION\ISR\isr.h>
#include <APPLICATION\ADS1256\ADS1256Ext.h>

// puntatore al buffer dei dati in ingresso per FFT
// viene inizializzato nella funzione process, allo stato
//IRAM_ATTR float *pInput;

// ISR per la gestione dell'arrivo di un nuovo campione sul fronte di discesa di DRDY
// TBD: 
//     rimuovere i "semafori"
//         newData = true;
//         countData++;
//     e portare qui l'acquisizione del campione grezzo 
//       adcValue = (float)adc.ReadRawData();
//     ed inserirlo in una coda che verrà letta dalla fsm del task process_task (parte da implementare anche là)
void IRAM_ATTR ISR_DRDY() {
  BaseType_t xHigherPriorityTaskWoken;

  newData = true;
  countData++;

  // push new data into ADS1256 queue
  xQueueSendFromISR( xQueueADS1256Sample, (void*)&countData, &xHigherPriorityTaskWoken );

  // Now the buffer is empty we can switch context if necessary
    if( xHigherPriorityTaskWoken )
    {
        /* Actual macro used here is port specific. */
        portYIELD_FROM_ISR();
    }
}