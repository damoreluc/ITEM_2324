#include <APPLICATION/FSM/fsm.h>
#include <APPLICATION\TASK\task.h>
#include <APPLICATION\HWCONFIG\hwConfig.h>
#include <MQTT\mqtt_functions.h>
#include <APPLICATION/FFT/FFT_data.h>
#include <APPLICATION\ADS1256\ADS1256Ext.h>
#include <APPLICATION\MCP3204\mcp3204.h>
#include <APPLICATION\MCP3204\mcp3204_data.h>
#include <APPLICATION\MAX31865\rtd_data.h>
#include <APPLICATION\SIM\sim_real_data_selector.h>
#include <APPLICATION\MCP6S26\MCP6S26.h>
#include <APPLICATION\MCP6S26\pga.h>
#include <APPLICATION\ISR\isr.h>

static uint64_t last = millis();
static uint64_t lastFFT;
static uint16_t sampleCounter = 0; // number of samples gathered
static uint32_t numberOfAds1256Cycles = 0;
static uint32_t countADCTorque = 0; // conteggio accessi ADC sensori di coppia
float adcValue;
int kk;

// some additional informations on task execution
uint32_t freeHeap = 0;
uint32_t elapsedTime = 0;

tMode triggered = Stop;

// flag true quando i dati sono pronti
volatile bool dataReady = false;

// stato della MSF
volatile tStati _stato = WaitTrigger;

// stampa stato attuale ---------------------------------------------------------------------------
void DebugCurrentStatus(tStati st)
{
    static tStati pst = Sampling; // initialize with any value different from WaitTrigger
    if (st != pst)
    {
        Serial.printf("_stato: %d\n", st);
        pst = st;
    }
}

// MSF del task di acquisizione e trasformazione dei dati -----------------------------------------
void fsm()
{
    switch (_stato)
    {
    case StartADC:
        // reset conteggio ADC torque
        countADCTorque = 0;
        dataReady = false;
        // imposta il canale corrente dell'ADC
        // Serial.printf("Channel: %d\n", MCP6S26_current_channel_index);

        // inizializzazione fft
        if (real_fft_plan == NULL)
        {
            Serial.println(F("Inizializzazione FFT"));
            // real_fft_plan = fft_init(FFT_SIZE, FFT_REAL, FFT_FORWARD, f_input, f_output);
            real_fft_plan = fft_init(FFT_SIZE, FFT_REAL, FFT_FORWARD, NULL, NULL);
            pInput = real_fft_plan->input;
        }

        last = millis();
        lastFFT = last;
        countADCTorque = 0;
        mcp3204_ResetBuffer();

        if (getSensMode() == REAL_DATA)
        {
            // set isr pointer at the beginning of real_fft_plan->input[] array:
           // pSampleISR = &real_fft_plan->input[0];

            // modalità SPI per transazioni con MCP3204 e MCP6S26
            vspi.beginTransaction(SPISettings(MCP3204_SPI_CLOCK, MSBFIRST, SPI_MODE0));

            //  uso il MUX del pga MCP6S26 per il multiplexing dei due ingressi accelerometrici verso ADS1256
             pga0.channel = pga0_channels[MCP6S26_current_channel_index];
             mcp6s26_setChannel(vspi, CS_PGA0, pga0.channel);
            // eventuale aggiornamento del guadagno del pga MCP6S26
            if (pga0.gain_changed)
            {
                pga0.gain_changed = false;
                mcp6s26_setGain(vspi, CS_PGA0, pga0.gain);
                Serial.print("\nPGA0 gain bits: ");
                Serial.println(pga0.gain);

                delay(5);
            }

            // avvio dell'ADC ADS1256 per acquisizione accelerometri
            adc.wakeup();
            // associa l'interrupt esterno di nDRDY alla sua ISR
            attachInterrupt(nDRDY, ISR_DRDY, FALLING);
        }

        // prossimo stato
        _stato = Sampling;

        break;

    case Sampling:
        // acquisizione di un canale accelerometrico dall'ADC ADS1256
        if (getSensMode() == REAL_DATA)
        {
            int32_t sampleFromISR;
            // while ((xQueueReceive(xQueueADS1256Sample, (void *)&sampleFromISR, 0) == pdTRUE) && (sampleCounter < FFT_SIZE))
            if (newData) // settato dalla ISR su DRDY dell'ADS1256
            {
                // sampleFromISR = real_fft_plan->input[sampleCounter];

                // Serial.print("Sample n. ");
                // Serial.print(sampleCounter);
                // Serial.print("  Value: ");
                // Serial.println(sampleFromISR);

                newData = false;

                // if (sampleCounter < FFT_SIZE)
                if (uxQueueSpacesAvailable(xQueueADS1256Sample) > 0)
                {
                    // get ADS1256 new sample
                    // adcValue = (float)adc.ReadRawData();
                    // real_fft_plan->input[sampleCounter] = adc.volt(adcValue);
                    ////real_fft_plan->input[sampleCounter] = (sampleFromISR % 75) * 5.0 / FFT_SIZE;
                    sampleCounter++;
                    numberOfAds1256Cycles++;

                    // get MCP3204 new samples
                    // if ((numberOfAds1256Cycles >= MCP3204_NUMBER_OF_ADS1256_CYCLES) && (mcp3204_BufferAvailable() > 0))
                    if ((numberOfAds1256Cycles >= MCP3204_NUMBER_OF_ADS1256_CYCLES))
                    {
                        numberOfAds1256Cycles = 0;
                        countADCTorque++;

                        // wake-up the MCP3204 acquisition task
                        xTaskNotifyGive(sampleMCP3204TaskHandle);
                    }
                }
                else
                {
                    // detachInterrupt(nDRDY);
                    //  ferma il campionamento al completamento del numero di campioni
                    adc.standby();

                    // termina la transazione SPI conl'adc MCP3204
                    vspi.endTransaction();

                    // pop samples from queue
                    for (uint16_t i = 0; i < FFT_SIZE; i++)
                    {
                        xQueueReceive(xQueueADS1256Sample, (void *)&sampleFromISR, 0);
                        // real_fft_plan->input[i] = (sampleFromISR % 75) * 5.0 / FFT_SIZE;
                        real_fft_plan->input[i] = (adc.volt(sampleFromISR)-2.5) * window[i];
                        // Serial.print("Sample n. ");
                        // Serial.print(i);
                        // Serial.print("  Value: ");
                        // Serial.println(sampleFromISR);
                    }

                    // // debug: campioni persi?
                    // Serial.print("campioni memorizzati: ");
                    // Serial.print(sampleCounter);
                    // Serial.print("   campioni acquisiti: ");
                    // Serial.print(countData - 1);
                    // Serial.print("   differenza: ");
                    // Serial.println(countData - 1 - sampleCounter);

                    // resetta l'indice dell'array dei dati
                    sampleCounter = 0;
                    countData = 0;

                    // reset the xQueueADS1256Sample queue
                    xQueueReset(xQueueADS1256Sample);

                    // segnala la fine del campionamento alla loop()
                    dataReady = true;
                }
            }
        }

        // simulazione campionamento per FFT (durata 576ms)
        else if (getSensMode() == SYM_DATA)
        {
            // dati ADS1256 simulati per singolo canale
            for (kk = 0; kk < FFT_SIZE; kk++)
            {
                real_fft_plan->input[kk] = 1.0 + 0.5 * MCP6S26_current_channel_index;
            }

            // dati coppie e velocità simulati
            for (kk = 0; kk < MCP3204_NUMBER_OF_SAMPLES_PER_CHANNEL; kk++)
            {
                mcp3204_dati.volt0 = 0.5;
                mcp3204_dati.volt1 = 1.0;
                mcp3204_dati.volt2 = 1.5;
                mcp3204_dati.volt3 = 2.0;
                mcp3204_Push(&mcp3204_dati);
            }

            // durata del ciclo di acquisizione reale
            delay(546);
            dataReady = true;
        }

        // termine dello stato di campionamento
        if (dataReady == true)
        {
            dataReady = false;
            _stato = Compute;
        }

        break;

    case Compute:
        // hang here until FFT output data are published
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // Execute FFT transformation
        fft_execute(real_fft_plan);

        // read RTD1 and store result into xQueueRTD1
        //++ dati RTD simulati
        readRTD();

        // push count ADC into xQueueCountAdcTorque
        if (uxQueueSpacesAvailable(xQueueCountADCTorque) > 0)
        {
            xQueueSendToBack(xQueueCountADCTorque, (void *)&countADCTorque, portMAX_DELAY);
        }

        _stato = Publish;
        break;

    case Publish:
        MCP6S26_publish_channel_index = MCP6S26_current_channel_index;
        // wake-up the publish task
        xTaskNotifyGive(publishTaskHandle);

        freeHeap = ESP.getFreeHeap();

        // passa al prossimo canale
        MCP6S26_current_channel_index++;

        // torna allo stato WaitTrigger
        _stato = WaitTrigger;
        break;

    case WaitTrigger:

        // print some task info
 //       Serial.printf("Free heap: %d bytes \t", freeHeap);
 //       Serial.printf("Elapsed time: %d ms", elapsedTime);

        // stay here until next Trigger command received by onMqttMessage
        if (mqttClient.connected() && (triggered == OneShot))
        {
            if (MCP6S26_current_channel_index < CHANNELS_N)
            {
                _stato = StartADC;
            }
            else
            {
                triggered = Stop;
                MCP6S26_current_channel_index = 0;
            }
        }
        else if (mqttClient.connected() && (triggered == FreeRun))
        {
            // passa al prossimo canale
            if (MCP6S26_current_channel_index >= CHANNELS_N)
            {
                MCP6S26_current_channel_index = 0;
            }
            _stato = StartADC;
        }
        else if (mqttClient.connected() && (triggered == Stop))
        {
            // termina la scansione
            MCP6S26_current_channel_index = 0;
            _stato = WaitTrigger;
        }
        delay(5);

        break;

    default:
        _stato = WaitTrigger;
        break;
    }
}
