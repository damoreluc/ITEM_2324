/*
 * ITEM firmware (ESP32 + AsyncMqttClient).
 *
 * Flusso di boot attuale:
 * 1) setup() avvia WiFiManager in modalita' bloccante per ottenere/aggiornare:
 *    - credenziali WiFi
 *    - broker MQTT, porta, username, password, client ID
 * 2) quando il provisioning termina, i parametri sono disponibili e persistiti.
 * 3) vengono inizializzati periferiche, code FreeRTOS e task applicativi.
 * 4) configMqttClient() usa i parametri runtime e allinea anche il prefisso topic al client ID.
 * 5) initWiFi_STA() usa la configurazione WiFi persistita e gestisce gli eventi essenziali.
 */

// Includes minimal libraries required
#include <Arduino.h>
#include <APPLICATION\HWCONFIG\hwConfig.h>
#include <WiFi.h>
#include <WIFI/wifi_functions.h>
#include <MQTT/mqtt_functions.h>
#include <MQTT/custom/custom.h>

// other libraries required by the specific application
#include <Wire.h>
#include <SPI.h>
#include <APPLICATION/ADS1256/ADS1256Ext.h>
#include <APPLICATION/ADS1256/ADS1256_equalizer.h>
#include <Adafruit_MAX31865.h>
#include <APPLICATION/MAX31865/rtd_MAX31865.h>
#include <APPLICATION\MAX31865\rtd_data.h>
#include <APPLICATION/MCP3204/mcp3204.h>
#include <APPLICATION\MCP3204\mcp3204_data.h>
#include <APPLICATION/MCP6S26/MCP6S26.h>
#include <APPLICATION/MCP6S26/pga.h>
#include <APPLICATION/SSD1306/ssd1306.h>
// #include <APPLICATION/FFT/FFT.h>
#include <APPLICATION/FFT/FFT_signal.h>
#include <APPLICATION/FFT/FFT_data.h>
// #include <APPLICATION/FFT/fast_sqrt.h>
#include <APPLICATION/SIM/sim_real_data_selector.h>
#include <APPLICATION/FSM/fsm.h>
#include <APPLICATION/TASK/task.h>
#include <APPLICATION/ISR/isr.h>
#include <APPLICATION/boot.h>
#include <APPLICATION/AUXDIN/aux_din.h>
#include <debug/debug_task.h>

TaskHandle_t debugTaskHandle = NULL;
stMqttRuntimeConfig mqttRuntimeConfig = {};
static const char *defaultMqttClientId = "ItemCV";

// uncomment this #define to print fft components
// #define PRINT_COMPONENTS

// MQTT defaults used as first-boot fallback for WiFiManager.
// Keep only one include enabled.
#include <MQTT/broker/shiftr_io.h>
// #include <MQTT/broker/raspi4.h>
// #include <MQTT/broker/mosquitto.h>


// -----------------------------------------------------------------------
void setup()
{
  // Captures the SENS_MODE pin and updates the simulated_data/real_data status
  readSensMode();

  Serial.begin(115200);

  // Run WiFiManager first (blocking) before allocating the rest of application memory.
  if (!runWiFiManagerBlocking(mqttRuntimeConfig,
                              mqttServer,
                              (uint16_t)mqttPort,
                              mqttUser,
                              mqttPassword,
                              defaultMqttClientId))
  {
    Serial.println(F("[BOOT] WiFiManager provisioning failed"));
    while (1)
    {
      delay(1000);
    }
  }

  bootMsg(mqttRuntimeConfig.broker, subscribedTopics, publishedTopics);

  // Force WiFi stack to run on core 0 (PRO_CPU)
  Serial.println(F("Configuring network stack for core 0..."));
  WiFi.setTxPower(WIFI_POWER_19_5dBm);  // Set TX power to reduce interference
  
  // creation of queue for ADS1256 ADC data acquisition from ISR
  xQueueADS1256Sample = xQueueCreate(ADS1256QueueSize, sizeof(int32_t));

  // creation of queue for MCP3204 ADC data publication
  xQueueCountADCTorque = xQueueCreate(3, sizeof(uint32_t));

  // creation of queue for RTD temperature publication
  xQueueRTD = xQueueCreate(6, sizeof(stRTD));

  if (getSensMode() == REAL_DATA)
  {
    // configure AUX_DIN as digital input
    pinMode(AUX_DIN, INPUT_PULLUP);
    // read 1-st input status
    aux_din_status = !digitalRead(AUX_DIN);

    // create ADS1256 equalization table
    ssd1306_log_setup();
    ssd1306_publish("Create EQ table\n");
    Serial.println(F("Create ADS1256 equalization table"));
    create_equalizer(m);
    ssd1306_publish("Create window\n");
    Serial.println(F("Create FFT window table"));
    welch(window, FFT_SIZE);    

    // initialise vspi with default pins
    // SCLK = 18, MISO = 19, MOSI = 23, SS = 5
    ssd1306_publish("Init vspi\n");
    vspi.begin();

    // set MCP3204 Chip select line
    ssd1306_publish("Setup MCP3204\n");
    pinMode(CS_MCP3204, OUTPUT);
    digitalWrite(CS_MCP3204, HIGH);

    // setup RTD1 and RTD object: set to 2WIRE, 3WIRE or 4WIRE as necessary
    ssd1306_publish("Setup RTD1/2\n");
    setupRTD();

    delay(500);

    // Initial ADC Accelerometer Setup ADS1256
    // sets the ISR of the ADS1256 ADC, triggered by the falling edge of data ready signal
    // Configuration of the ADS1256 and its control lines
    //  NB: HSPI clock <= F_clkin / 4 = 7.68e6 / 4 = 1920000
    ssd1306_publish("Setup ADS1256\n");
    pinMode(nDRDY, INPUT_PULLUP);
    adc.setup();

    Serial.println(F("PGA MCP6S26 Configuration"));
    ssd1306_publish("Setup PGA\n");

    // PGA initial setup PGA
    mcp6s26_setup();

    Serial.println(F("ADC & PGA Configuration Completed"));

    ssd1306_publish("Create FFT tasks\n");
  }
  else if (getSensMode() == SYM_DATA)
  {
    Serial.println("Simulated data, without sensors and without display");
  }

  // Creating and Starting Tasks
  BaseType_t xReturned;

  // create the FFT evaluation task
  xReturned = xTaskCreatePinnedToCore(
      process,            // function that implements the task
      "process",          // name for the task
      4096,               // task size
      NULL,               // parameter passed into the task
      4,                  // task priority
      &processTaskHandle, // the task's handle
      APP_CPU_NUM         // pinned to core 0/1
  );

  if (xReturned != pdPASS)
  {
    Serial.println(F("Error creating the task FFT"));
    while (1)
    {
      if (getSensMode() == REAL_DATA)
      {
        ssd1306_publish("Error on FFT task\n");
      }
      yield();
    }
  }

  // create the FFT data publishing on MQTT task
  xReturned = xTaskCreatePinnedToCore(
      publishFFT,         // function that implements the task
      "publishFFT",       // name for the task
      4096,               // task size
      NULL,               // parameter passed into the task
      6,                  // task priority
      &publishTaskHandle, // the task's handle
      PRO_CPU_NUM         // pinned to core 0/1 (PRO_CPU_NUM)
  );

  if (xReturned != pdPASS)
  {
    Serial.println(F("Error creating the task publishFFT"));
    while (1)
    {
      if (getSensMode() == REAL_DATA)
      {
        ssd1306_publish("Error on publish task\n");
      }
      yield();
    }
  }

  // Create the MCP3204 acquisition task
  xReturned = xTaskCreatePinnedToCore(
      sampleMCP3204,            // function that implements the task
      "sampleMCP3204",          // name for the task
      2048,                     // task size
      NULL,                     // parameter passed into the task
      5,                        // task priority
      &sampleMCP3204TaskHandle, // the task's handle
      APP_CPU_NUM               // pinned to core 1
  );

  if (xReturned != pdPASS)
  {
    Serial.println(F("Error creating the task sampleMCP3204"));
    while (1)
    {
      if (getSensMode() == REAL_DATA)
      {
        ssd1306_publish("Error on sampleMCP3204\n");
      }
      yield();
    }
  }

  // Create periodic debug diagnostic task
  xReturned = xTaskCreatePinnedToCore(
      debugTask,        // function that implements the task
      "debugTask",     // name for the task
      2048,             // task size
      NULL,             // parameter passed into the task
      1,                // task priority
      &debugTaskHandle, // the task's handle
      PRO_CPU_NUM       // pinned to core 0
  );

  if (xReturned != pdPASS)
  {
    Serial.println(F("Error creating the task debugTask"));
    while (1)
    {
      yield();
    }
  }

  //-----------------------------------------------------------------------
  // employs the macAddress() method of the WiFi object
  Serial.println();
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (getSensMode() == REAL_DATA)
  {
    ssd1306_publish("MAC: ");
    ssd1306_publish(WiFi.macAddress().c_str());
  }

  // mqtt client configuration
  configMqttClient(mqttRuntimeConfig.broker,
                   mqttRuntimeConfig.port,
                   mqttRuntimeConfig.user,
                   mqttRuntimeConfig.password,
                   mqttRuntimeConfig.clientId);

  // initiates the connection to an access point and registers WiFi event handlers;
  // with the GotIP event, connections are initiated:
  // to the NTP server
  // to the MQTT broker
  initWiFi_STA();

  // Unlock the processing task, the output array is free
  xTaskNotifyGive(processTaskHandle);

  // remove the setup() and loop() task
  vTaskDelete(NULL);
}

void loop()
{

}