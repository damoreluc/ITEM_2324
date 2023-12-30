/*
  Sostituiti i dizionari dei topics (published e subscribed)
  con array statici di char
  per valutare la causa dei kernel panic random 
*/

/*
 * WiFi Esempio 05: template di client MQTT con connessione a rete WiFi
 *
 * Il progetto è un modello base per realizzare un client MQTT. Va considerato come riferimento
 * per progetti più articolati basati sul protocollo MQTT su ESP32.
 *
 * Impiega la libreria AsyncMqttClient.
 *
 * L'esempio accetta quattro subscribed topics:
 *  ESP32_base/yellowTopic  per il comando del led giallo
 *  ESP32_base/redTopic     per il comando del led rosso
 *  ESP32_base/blueTopic    per il comando del led blu
 *  ESP32_base/input        per la stampa di un messaggio di testo sulla console seriale
 *
 * e pubblica sul topic:
 *  ESP32_base/output
 * un testo che rappresenta lo stato attuale di un pulsante.
 *
 * CONFIGURAZIONE WIFI ------------------------------------------------------------------------------
 * La scheda ESP32 è configurata come STATION in una rete WiFi:
 * modificare il file WIFI/credentials.h con i propri SSID e Password
 * Per il controllo dello stato della connessione vengono adoperati gli eventi dell'oggetto Wifi
 * Vengono gestite le situazioni di perdita di connessione mediante riconnessione automatica.
 * Il pin 23 viene impiegato come uscita digitale per indicare la connessione all'access point wiFi.
 *
 * SINCONIZZAZIONE DATA/ORA CON NTP SERVER ----------------------------------------------------------
 * Viene utilizzato un server NTP per la gestione di data e ora:
 * modificare il file LOCALTIME/localTime.cpp con i propri parametri NTP
 *
 * CONFIGURAZIONE DEL BROKER MQTT DA IMPIEGARE ------------------------------------------------------
 * Sono predefiniti tre broker MQTT tramite i file di configurazione:
 *   MQTT/broker/mosquitto.h
 *   MQTT/broker/raspi4.h
 *   MQTT/broker/shiftr_io.h
 *
 * ed è possibile aggiungerne altri rispettando il formato del file ed il nome delle variabili:
 *
 *   #ifndef __NOMEBROKER_H
 *   #define __NOMEBROKER_H
 *     // parametri di accesso per il broker NOMEBROKER MQTT
 *     const char *mqttServer = "IP_o_url_del_broker_MQTT";
 *     const int mqttPort = 1883;
 *     const char *mqttUser = "eventuale_username_di_accesso_al_broker";
 *     const char *mqttPassword = "eventuale_password_di_accesso_al_broker";
 *   #endif
 *
 * Nel file main.cpp includere il solo file prescelto di accesso al broker, ad esempio:
 *    // your MQTT Broker:
 *    // uncomment one of following #include to set the MQTT broker.
 *    #include <MQTT/broker/shiftr_io.h>
 *    // #include <MQTT/broker/raspi4.h>
 *    // #include <MQTT/broker/mosquitto.h>
 *
 * IMPORTANTE: NON includere più di un file di definizione del broker
 *
 *
 * DEFINIZIONE DEI SUBSCRIBED E DEI PUBLISHING TOPICS ------------------------------------------
 *
 * In base all'applicazione da creare, vanno definiti i topics a cui il client deve iscriversi
 * per ricevere dati o comandi da remoto.
 *
 * Il client MQTT deve possedere un nome univoco sul broker.
 * Nel file MQTT\custom\mqtt_topics.h
 * viene dichiarato il nome univoco, che è possibile personalizzare:
 *    // MQTT client ID
 *    #define thisClient "ESP32_base"
 *
 *
 * Vanno poi definiti i topic sui quali il client pubblica dati verso il broker.
 * Allo scopo vengono impiegati due dizionari:
 *   per i subscribed topics ("ingressi" per il client):  subscribedTopics
 *   per i publishing topics ("uscite" del client):  publishedTopics
 *
 * entrambi i dizionari hanno la struttura:  <chiave>, <valore>
 * entrambe le voci sono di tipo String.
 * Con <chiave> si indica un nome semplice da assegnare al topic
 * Il <valore> contiene il persorso logico del topic.
 *
 *
 * La personalizzazione va svolta nel file MQTT\custom\mqtt_topics.cpp
 * Per i topic in ingresso modificare la funzione:
 *    void compileSubTopics(Dictionary<String, String> &subTopics)
 *
 * Per i topic in uscita modificare la funzione:
 *    void compilePubTopics(Dictionary<String, String> &pubTopics)
 *
 *
 * Procedere nell'ordine:
 * 1. definire quali informazioni la scheda ESP32 dovrà ricevere dal broker
 *    e associare i subscribed topics, definendo un nome univoco per ciascun topic
 *    e il suo percorso sul broker. Ad esempio:
 *      si vuole accendere/spegnere da remoto un led giallo collegato alla ESP32;
 *      si definisce un subscribed topic con nome "yellowTopic"
 *      e con percorso  "ESP32_base/yellowTopic"
 *      da questo topic arriverà la stringa "0" per spegnere il led, o la stringa "1" per
 *      accenderlo.
 *
 * 2. in modo simile, definire quali informazioni la scheda ESP32 pubblicherà verso il broker
 *    e associare i publisher topics, definendo un nome univoco per ciascun topic
 *    e il suo percorso sul broker. Ad esempio:
 *      si vuole notificare in remoto che un pulsante è stato premuto o rilasciato;
 *      si definisce un published topic con nome "outTopic"
 *      e con percorso  "ESP32_base/output"
 *      su questo topic la ESP32 invierà un messaggio sullo stato del pulsante
 *
 * 3. per i subscribed topics, nella funzione compilePubTopics() aggiungere il topic al dizionario
 *    dei subscribed topics mediante il comando:
 *      // subscribed topic di comando del led giallo
 *      subTopics.set("yellowTopic", thisClient "/yellowTopic");
 *
 * 4. ripetere il punto 3. per ciascun subscribed topic richiesto dalla applicazione
 *
 * 5. per i publishing topic, nella funzione compilePubTopics() aggiungere il topic al dizionario
 *    dei published topics mediante il comando:
 *      // topic di pubblicazione messaggi
 *      pubTopics.set("outTopic", thisClient "/output");
 *
 * 6. ripetere il punto 5. per ciascun published topic richiesto dalla applicazione.
 *
 * All'avvio del client MQTT verranno automaticamente registrati sul broker i subscribed topics.
 *
 *
 * PARSING DEI SUBSCRIBED TOPICS ---------------------------------------------------------------
 * Il client MQTT gestisce il traffico col broker in modo asincrono, non è necessario che
 * il programmatore della applicazione si preoccupi delle fasi di ricezione o di trasmissione
 * dei dati sui topics.
 *
 * E' invece responsabilità del programmatore decidere cosa fare quando viene ricevuto un messaggio
 * su un subscribed topic.
 * Nel file     MQTT\custom\parseMessage.cpp
 * va personalizzata la funzione    parseMessage()
 *
 * Rifacendosi al caso del led giallo illustrato più sopra, nella parseMessage() si andrà a
 * 1. verificare il topic del messaggio ricevuto
 * 2. se si tratta del topic descritto dalla chiave "yellowTopic" allora si valuta il contenuto del payload
 * 3. se il payload è la stringa "0" viene comandato lo spegnimento del led giallo
 *    altrimenti se il payload è la stringa "1" viene comandata l'accensione del led giallo
 *
 * La struttura del codice corrispondente è:
 *
 *  // comando del led giallo
 *  // è arrivato un messaggio da yellowTopic
 *  if (strcmp(topic, subscribedTopics.get("yellowTopic").c_str()) == 0)
 *  {
 *    if (strncmp(payload, "0", 1) == 0)
 *    {
 *      digitalWrite(pinYellow, LOW);
 *      Serial.println("led giallo spento");
 *    }
 *    else if (strncmp(payload, "1", 1) == 0)
 *    {
 *      digitalWrite(pinYellow, HIGH);
 *      Serial.println("led giallo acceso");
 *    }
 *  }
 *
 * NOTA: nelle operazioni di confronto tra stringhe del payload (trattate come array di char)
 *       è consigliato utilizzare la funzione di confronto strncmp() specificando esattamente
 *       il numero di caratteri da confrontare.
 *
 * PUBBLICAZIONE DI UN DATO SUI PUBLISHING TOPICS ---------------------------------------------
 * Anche la pubblicazione di un dato dalla ESP32 verso il broker viene gestita in modalità
 * asincrona dai layers della libreria MQTT.
 * Nella applicazione è sufficiente che il programmatore utilizzi il metodo mqttClient.publish()
 * ogni qual volta desidera pubblicare una informazione su un particolare topic.
 *
 * Il metodo mqttClient.publish() richiede i seguenti parametri:
 *   il topic sul quale pubblicare, ad esempio "ESP32_base/output"
 *   il livello di QOS (ad esempio 0)
 *   se il messaggio è di tipo retain o no
 *   il payload da trasmettere (ad esempio un array di char contenente un testo)
 *   la dimensione in byte del payload
 *
 * e ritorna il packet ID (diverso da 0) se è stato in grado di inserire il payload nella
 * coda dei messaggi da pubblicare verso il broker, altrimenti restituisce il codice di
 * errore 0.
 *
 * Ad esempio, per pubblicare lo stato del pulsante, il programmatore può scrivere:
 *
 *   if (button.fell())
 *   {
 *     const char msgButton[] = "Pulsante premuto";
 *     Serial.println(msgButton);
 *
 *     // pubblica sul topic outTopic
 *     if(mqttClient.connected()) {
 *       uint16_t res = 0;
 *       res = mqttClient.publish(publishedTopics.get("outTopic").c_str(),0,false, msgButton, strlen(msgButton));
 *     }
 *   }
 *
 * Si osservi che il payload (l'informazione inviata sul canale definito dal topic) verrà trattata
 * dai layers di basso livello come array di byte. Pertanto il payload può anche essere un int, un float
 * o un tipo dati definito dall'utente, purché sia determinabile la sua dimensione in byte e che
 * il tipo dati sia "riconoscibile" e "gestibile" da chi riceverà l'informazione dall'altro lato del broker.
 *
 *
 * FUNZIONE SETUP() ---------------------------------------------------------------------------
 * Nella funzione setup() è importante rispettare la sequenza di operazioni:
 * 1. configurare tutti i dispositivi hardware
 * 2. assegnare i valori predefiniti a variabili/oggetti della applicazione
 * 3. creare eventuali task RTOS
 * 4. configurare il client MQTT con configMqttClient(mqttServer, mqttPort, mqttUser, mqttPassword);
 * 5. avviare il sotto sistema WiFi con initWiFi_STA();
 *
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

// uncomment this #define to print fft components
// #define PRINT_COMPONENTS

// your MQTT Broker: -----------------------------------------------
// uncomment one of following #include to set the MQTT broker.
#include <MQTT/broker/shiftr_io.h>
// #include <MQTT/broker/raspi4.h>
// #include <MQTT/broker/mosquitto.h>

// ADS1256 parameters ----------------------------------------------

// canale SPI per connessione con ADC ADS1256
// SPIClass hspi = SPIClass(HSPI);

// // ADC equalization table - see create_equalizer(m)
// float m[FFT_SIZE >> 1];

// // ADS1256 DRDY
// volatile bool newData = false;
// volatile uint16_t countData = 0;

// // ADC instance
// ADS1256Ext adc;
// // list of channels to sample
// // byte channels[CHANNELS_N] = {adc.ads1256_mux[0], adc.ads1256_mux[1]};
// byte channels[CHANNELS_N] = {adc.ads1256_mux[0], adc.ads1256_mux[0]}; // mux now is located into PGA0

// using differential inputs:
//  AIN0+ as signal+
//  AIN1+ as ADC_Half_Vref
// byte channels[CHANNELS_N] = {adc.ads1256_dmux[0], adc.ads1256_dmux[0]}; // using PGA0 also as AMUX



// -----------------------------------------------------------------------
void setup()
{
  // Captures the SENS_MODE pin and updates the simulated_data/real_data status
  readSensMode();

  Serial.begin(115200);
  bootMsg(mqttServer, subTopics, pubTopics);

  // creation of queue for ADS1256 ADC data acquisition from ISR
  xQueueADS1256Sample = xQueueCreate(ADS1256QueueSize, sizeof(int32_t));

  // creation of queue for MCP3204 ADC data publication
  xQueueCountADCTorque = xQueueCreate(2, sizeof(uint32_t));

  // creation of queue for RTD temperature publication
  xQueueRTD = xQueueCreate(5, sizeof(stRTD));

  if (getSensMode() == REAL_DATA)
  {
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
      10,                 // task priority
      &processTaskHandle, // the task's handle
      APP_CPU_NUM         // pinned to core 1
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
      1,                  // task priority
      &publishTaskHandle, // the task's handle
      APP_CPU_NUM         // pinned to core 0 (PRO_CPU_NUM)
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
      11,                        // task priority
      &sampleMCP3204TaskHandle, // the task's handle
      APP_CPU_NUM               // pinned to core 0
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
  configMqttClient(mqttServer, mqttPort, mqttUser, mqttPassword);

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