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
float meanValue = 0.0;
int kk;

// some additional informations on task execution
uint32_t freeHeap = 0;
uint32_t elapsedTime = 0;
uint32_t QueueLength, maxQueueLength = 0;

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

// FSM of the Data Capture and Transformation Task ------------------------------------------------
void fsm()
{
    switch (_stato)
    {
    case StartADC:
        // ADC Torque Count Reset
        countADCTorque = 0;
        dataReady = false;
        // sets the current channel of the ADC
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

            // SPI mode for MCP3204 and MCP6S26 transactions
            vspi.beginTransaction(SPISettings(MCP3204_SPI_CLOCK, MSBFIRST, SPI_MODE0));

            // I use the MUX of the pga MCP6S26 to multiplexe the two accelerometer inputs to ADS1256
            pga0.channel = pga0_channels[MCP6S26_current_channel_index];
            mcp6s26_setChannel(vspi, CS_PGA0, pga0.channel);
            // Possible update of the PGA gain MCP6S26
            if (pga0.gain_changed)
            {
                pga0.gain_changed = false;
                mcp6s26_setGain(vspi, CS_PGA0, pga0.gain);
                Serial.print("\nPGA0 gain bits: ");
                Serial.println(pga0.gain);

                delay(5);
            }

            // Starting the ADC ADS1256 for Accelerometer Acquisition
            adc.wakeup();
            // binds nDRDY's external interrupt to its ISR
            attachInterrupt(nDRDY, ISR_DRDY, FALLING);
        }

        // Move to the next status
        _stato = Sampling;
        // reset counter samples captured from the xQueueADS1256Sample queue
        sampleCounter = 0;

        break;

    case Sampling:
        // Acquiring Accelerometer samples from the ADS1256 ADC
        // provare usare taskNotify per sincronizzare l'acquisizione tra la ISR e la FSM nello stato Sampling
        if (getSensMode() == REAL_DATA)
        {
            int32_t sampleFromISR;

                // Serial.print("Sample n. ");
                // Serial.print(sampleCounter);
                // Serial.print("  Value: ");
                // Serial.println(sampleFromISR);

                if (sampleCounter < FFT_SIZE)
                {
                    QueueLength = uxQueueMessagesWaiting(xQueueADS1256Sample);
                    // if (uxQueueSpacesAvailable(xQueueADS1256Sample) > 0)
                    if (QueueLength > 0)
                    {
                        // pop ADS1256 samples from queue
                        adcValue = adc.volt(adc.ReadRawData());
                        meanValue += adcValue;

                        xQueueReceive(xQueueADS1256Sample, (void *)&sampleFromISR, 5);

                        // real_fft_plan->input[sampleCounter] = (sampleFromISR % 75) * 5.0 / FFT_SIZE;
                        // real_fft_plan->input[sampleCounter] = (adc.volt(sampleFromISR) - 2.5) * window[sampleCounter];
                        real_fft_plan->input[sampleCounter] = adcValue;

                        // update sample counter
                        sampleCounter++;
                        numberOfAds1256Cycles++;
                        if (QueueLength > maxQueueLength)
                        {
                            maxQueueLength = QueueLength;
                        }

                        // get MCP3204 new samples
                        if ((numberOfAds1256Cycles >= MCP3204_NUMBER_OF_ADS1256_CYCLES))
                        {
                            numberOfAds1256Cycles = 0;
                            countADCTorque++;

                            // wake-up the MCP3204 acquisition task
                            xTaskNotifyGive(sampleMCP3204TaskHandle);
                        }
                    }
                }
                else
                {
                    // detachInterrupt(nDRDY);
                    // Stop sampling when the number of samples is reached
                    adc.standby();

                    // Terminate the SPI transaction with the MCP3204 ADC
                    vspi.endTransaction();

                    // Reset the Data Array Index
                    sampleCounter = 0;
                    countData = 0;

                    // Signal end of sampling
                    dataReady = true;
                }
            
        }

        // Sampling simulation for FFT (duration 576ms)
        else if (getSensMode() == SYM_DATA)
        {
            // Simulated ADS1256 data per channel
            for (kk = 0; kk < FFT_SIZE; kk++)
            {
                real_fft_plan->input[kk] = 1.0 + 0.5 * MCP6S26_current_channel_index;
            }

            // Simulated torque and speed data
            for (kk = 0; kk < MCP3204_NUMBER_OF_SAMPLES_PER_CHANNEL; kk++)
            {
                mcp3204_dati.volt0 = 0.5;
                mcp3204_dati.volt1 = 1.0;
                mcp3204_dati.volt2 = 1.5;
                mcp3204_dati.volt3 = 2.0;
                mcp3204_Push(&mcp3204_dati);
            }

            // Actual Acquisition Cycle Length
            delay(546);
            dataReady = true;
        }

        // End of Sampling Status
        if (dataReady == true)
        {
            dataReady = false;
            _stato = Compute;
        }

        break;

    case Compute:
        // hang here until FFT output data are published
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // remove mean value and apply windowing on time samples
        meanValue /= FFT_SIZE;
        for(uint16_t i = 0; i < FFT_SIZE; i++) {
            real_fft_plan->input[i] = (real_fft_plan->input[i] - meanValue) * window[i];
        }

        // Execute FFT transformation
        fft_execute(real_fft_plan);

        // read RTD1 and store result into xQueueRTD1
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

        // Skip to the next channel
        MCP6S26_current_channel_index++;

        // back to WaitTrigger status
        _stato = WaitTrigger;

        // print some task info
        Serial.printf("Free heap: %d bytes \t", freeHeap);
        Serial.printf("Max queue length: %d\n", maxQueueLength);
        maxQueueLength = 0;
        //       Serial.printf("Elapsed time: %d ms", elapsedTime);
        break;

    case WaitTrigger:

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
            // Skip to the next channel
            if (MCP6S26_current_channel_index >= CHANNELS_N)
            {
                MCP6S26_current_channel_index = 0;
            }
            _stato = StartADC;
        }
        else if (mqttClient.connected() && (triggered == Stop))
        {
            // End the Scan
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
