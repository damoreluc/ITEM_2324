#include <APPLICATION\TASK\task.h>
#include <APPLICATION/FFT/FFT_data.h>
#include <APPLICATION\ADS1256\ADS1256_equalizer.h>
#include <APPLICATION\FFT\fast_sqrt.h>
#include <APPLICATION\MAX31865\rtd_data.h>
#include <APPLICATION\MCP3204\mcp3204_data.h>
#include <MQTT\mqtt_functions.h>
#include <MQTT\custom\mqtt_topics.h>

// handle del task di pubblicazione FFT
TaskHandle_t publishTaskHandle;

// task pubblicazione FFT
void publishFFT(void *pvParameters)
{
  while (1)
  {
    // hang here until FFT is computed
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (real_fft_plan != NULL)
    {
      /*Multiply the magnitude of the DC component with (1/FFT_N) to obtain the DC component*/
      /* m[0] is the equalization coefficient */
      real_fft_plan->output[0] = real_fft_plan->output[0] * ONE_OVER_FFT_SIZE * m[0];
      /* channel error correction:
         by definition, all the FFT magnitude components are NON Negative value; we can safely change
         the sign of the DC component (the first array's element) to allow the receiver verify
         the correct sequence of the two halves sent over MQTT channel.
         In some cases the Node-RED HMI swaps the two data blocks and, as a consequence, the DC value
         is drawn at the half of the spectrum instead of the very first bin.
      */
      real_fft_plan->output[0] *= -1.0;

#ifdef PRINT_COMPONENTS
      // Print the output
      Serial.printf("Freq[Hz]\tAmpl[V]\n");
      sprintf(datapkt, "%7.2f\t%5.3f\n", 0.0, (f_output[0]));
      Serial.printf(datapkt);
#endif

      float a, b, c;

      for (int k = 1; k < real_fft_plan->size / 2; k++)
      {
        /*The real part of a magnitude at a frequency is followed by the corresponding imaginary part in the output*/
        a = real_fft_plan->output[2 * k];
        b = real_fft_plan->output[2 * k + 1];
        c = (a * a + b * b);

        float mag = Q_sqrt(c); // 1.59ms

        // store magnitude result in the lower half of output[] array
        // m[k] is the equalization coefficient
        real_fft_plan->output[k] = mag * 2.0 * ONE_OVER_FFT_SIZE * m[k];

#ifdef PRINT_COMPONENTS
        double freq = k * df;
        sprintf(datapkt, "%4d\t%7.2f\t%5.3f\n", k, freq, mag);
        // strcat(publishpkt, datapkt);
        // pos++;
        Serial.printf(datapkt);
#endif
      }

      // publish FFT data on MQTT  when connected to the MQTT broker...
      if (mqttClient.connected())
      {

        // publish the message
        uint16_t j;
        uint16_t res = 0;
        uint32_t begin = millis();

        for (j = 0; j < BLOCKS_FLOAT; j++)
        {
          do
          {
            if (MCP6S26_publish_channel_index == 0)
            {
              res = mqttClient.publish(publishedTopics.get("outTopic0").c_str(), 0, false, (const char *)&(real_fft_plan->output[BLOCK_FLOAT_HALF_SIZE * j]), MQTT_MAX_SIZE_BYTE);
            }
            else if (MCP6S26_publish_channel_index == 1)
            {
              res = mqttClient.publish(publishedTopics.get("outTopic1").c_str(), 0, false, (const char *)&(real_fft_plan->output[BLOCK_FLOAT_HALF_SIZE * j]), MQTT_MAX_SIZE_BYTE);
            }

            delay(25);
          } while (res == 0);
        }

        uint32_t now = millis();

        Serial.print("Elapsed time: ");
        Serial.print(now - begin);
        Serial.println(" ms");

        // pubblicazione degli eventuali dati delle RTD
        if (xQueueRTD != NULL)
        {
          stRTD temp;
          char s[10];

          while (uxQueueMessagesWaiting(xQueueRTD) > 0)
          {
            xQueueReceive(xQueueRTD, &(temp), portMAX_DELAY);

            // pubblicazione valore RTD1
            sprintf(s, "%.3f", temp.rtd1);
            do
            {
              res = mqttClient.publish(publishedTopics.get("outTopic2").c_str(), 0, false, (const char *)&s[0], strlen(s));
              delay(10);
            } while (res == 0);
            // pubblicazione stato RTD1
            do
            {
              res = mqttClient.publish(publishedTopics.get("outTopic3").c_str(), 0, false, (const char *)&temp.fault1[0], strlen(&temp.fault1[0])); // MSG_FAULT_LEN);
              delay(10);
            } while (res == 0);

            // pubblicazione valore RTD2
            sprintf(s, "%.3f", temp.rtd2);
            do
            {
              res = mqttClient.publish(publishedTopics.get("outTopic4").c_str(), 0, false, (const char *)&s[0], strlen(s));
              delay(10);
            } while (res == 0);
            // pubblicazione stato RTD2
            do
            {
              res = mqttClient.publish(publishedTopics.get("outTopic5").c_str(), 0, false, (const char *)&temp.fault2[0], strlen(&temp.fault2[0])); // MSG_FAULT_LEN);
              delay(10);
            } while (res == 0);
          }
        }

        // pubblicazione conteggio accessi ADC
        if (xQueueCountADCTorque != NULL)
        {
          uint32_t nadc;
          char s[10];

          while (uxQueueMessagesWaiting(xQueueCountADCTorque) > 0)
          {
            xQueueReceive(xQueueCountADCTorque, &nadc, portMAX_DELAY);
            sprintf(s, "%9d", nadc);
            do
            {
              res = mqttClient.publish(publishedTopics.get("outTopic6").c_str(), 0, false, (const char *)&s[0], strlen(s));
              delay(10);
            } while (res == 0);
          }
        }

        // pubblicazione dati mcp3204 mcp3204buffer[MCP3204_BUFFER_SIZE]
        do
        {
          res = mqttClient.publish(publishedTopics.get("outTopic7").c_str(), 0, false, (const char *)&mcp3204buffer[0], MCP3204_BUFFER_SIZE * sizeof(float));
          delay(10);
        } while (res == 0);
      }

      // sblocca il task di elaborazione, l'array output è libero
      xTaskNotifyGive(processTaskHandle);
    }
  }
}