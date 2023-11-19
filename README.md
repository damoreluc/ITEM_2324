# Progetto ITEM: <br>refactoring firmware per ESP32

## Repository del progetto ITEM - Banco prova riduttori meccanici

Progetto base: __Git_ESP32_Devkit_ADS1256_FFT_SIMUL_RTD_SSD1306_count_AsyncMQTT_binary_05__

### Firmware per ESP32

Il codice è sviluppato con il framework Arduino in ambiente VS Code/PlatformIO.

## La versione corrente supporta le seguenti funzionalità:

1. Acquisizione dei due ingressi analogici per [accelerometri piezo elettrici IEPE](https://www.allaboutcircuits.com/technical-articles/introduction-to-piezoelectric-accelerometers-with-integral-electronics-piezo-electric-iepe-sensor/)
2. Impiego dell'amplificatore a guadagno programmabile [MCP6S26](https://www.microchip.com/en-us/product/MCP6S26#) con multiplexer analogico per la selezione del canale accelerometrico e la regolazione del guadagno
3. Impiego del convertitore analogico digitale [ADS1256](https://www.ti.com/product/ADS1256) con risoluzione di 24 bit 
4. Campionamento di ciascun canale accelerometrico a 7500 Sa/s
5. Calcolo dello spettro delle ampiezze di ciascun canale accelerometrico su 4096 campioni, risoluzione in frequenza 1,83Hz
6. Acquisizione di 4 canali analogici con ampiezza +/-10V per i due segnali di coppia e i due segnali di velocità mediante ADC [MCP3204](https://www.microchip.com/en-us/product/MCP3204), risoluzione di 12 bit, frequenza di campionamento 100 Sa/s
7. Acquisizione di due segnali di temperatura tramite trasduttori RTD PT1000 e moduli di condizionamento e conversione [Adafruit PT1000 RTD Temperature Sensor Amplifier - MAX31865](https://www.adafruit.com/product/3648)
8. Visualizzazione locale dello stato di funzionamento tramite display OLED [SSD1306](https://nettigo.eu/system/images/3580/original.JPG?1578141142) con interfaccia I2C
9. Connessione alla rete dati mediante Wifi
10. Trasmissione dei dati telemetrici mediante protocollo [MQTT](https://www.hivemq.com/mqtt-essentials/) su broker a scelta: [mosquitto](https://test.mosquitto.org/) remoto o locale, [shiftr.io](https://www.shiftr.io/)
11. Gestione da remoto tramite messaggi MQTT

---

## Cartella `\APPLICATION`

La cartella `\APPLICATION` è pensata per organizzare meglio il codice del progetto.<br>Inserire qui le funzioni ausiliarie specifiche per l'applicazione in sviluppo, ad esempio le callback di accensione e spegnimento dei led in funzione del contenuto del payload.

---

## CONFIGURAZIONE WIFI

La scheda ESP32 è configurata come __STATION__ in una rete WiFi:

* modificare il file `WIFI/credentials.h` con i propri `SSID` e `Password`

Per il controllo dello stato della connessione vengono adoperati gli eventi dell'oggetto Wifi.

Vengono gestite le situazioni di perdita di connessione mediante riconnessione automatica.

~~Il pin `23` viene impiegato come uscita digitale per indicare la connessione all'access point wiFi. Per configurare un'altra posizione del led, modificare il parametro `pinWiFiConnected` nel file `HWCONFIG\hwConfig.h`~~

---

## SINCRONIZZAZIONE DATA/ORA CON NTP SERVER

Viene utilizzato un server __NTP__ per la gestione di data e ora:

* modificare il file `LOCALTIME/localTime.cpp` con i propri parametri NTP

Per la stampa di data e ora utilizzare la funzione `printLocalTime()`. 

---

## CONFIGURAZIONE DEL BROKER MQTT DA IMPIEGARE

Sono predefiniti tre broker MQTT tramite i rispettivi file di configurazione:

* `MQTT/broker/mosquitto.h`
* `MQTT/broker/raspi4.h`
* `MQTT/broker/shiftr_io.h`

ed è possibile aggiungerne altri rispettando il formato del file ed il nome delle variabili:

```C
#ifndef __NOMEBROKER_H
#define __NOMEBROKER_H
  // parametri di accesso per il broker NOMEBROKER MQTT
  const char *mqttServer = "IP_o_url_del_broker_MQTT";
  const int mqttPort = 1883;
  const char *mqttUser = "eventuale_username_di_accesso_al_broker";
  const char *mqttPassword = "eventuale_password_di_accesso_al_broker";
#endif
```

Nel file `main.cpp` includere __il solo__ file prescelto di accesso al broker, ad esempio:

```C
 // your MQTT Broker:
 // uncomment one of following #include to set the MQTT broker.
 #include <MQTT/broker/shiftr_io.h>
 // #include <MQTT/broker/raspi4.h>
 // #include <MQTT/broker/mosquitto.h>
```

## Identificativo MQTT della scheda ESP32

Nel file `src\MQTT\custom\clientID.h` è definita la tag `thisClient` che identifica tutti i messaggi MQTT pubblicati da o destinati a questa scheda di acquisizione. Modificarlo in base alle proprie esigenze.

```C
// MQTT client ID
#define thisClient "ESP32DevKit123" //"ESP32Udine"

```

## Elenco dei Topics MQTT impiegati

L'elenco di tutti i topics è dichiarato nel file `mqtt_topics.cpp`  
in questo file modificare o aggiungere solo il nome del topic, non rimuovere la tag **_thisClient_**

Ogni topic è nella forma sintattica: `identificativo_MQTT_scheda/nome_topic` ad esempio: `ESP32DevKit123/trigger`

**Nota:** il testo dei topics è _case sensitive_.

Il controllo remoto della ESP32 avviene tramite i seguenti _subscribed topics_ :

| nome_topic | Tipo | Valori | Descrizione |
|:--:|:---:|:---:|----|
| /trigger | stringa | 0, 1, 2 | Controlla la MSF delle acquisizioni. <ul><li>0: Fine acquisizione</li><li>1: Singola acquisizione</li><li>2: Acquisizione ciclica</li> </ul>|
| /pgaSetGain | stringa | 0..7 | Controlla il guadagno del PGA: <ul><li>0: Guadagno x 1</li><li>1: Guadagno x 2</li><li>2: Guadagno x 4</li><li>3: Guadagno x 5</li><li>4: Guadagno x 8</li><li>5: Guadagno x 10</li><li>6: Guadagno x 16</li><li>7: Guadagno x 32</li> </ul>|

La scheda ESP32 comunica la telemetria attraverso i seguenti _published topics_ :

| nome_topic | Tipo | Note | Descrizione |
|:--:|:---:|:---:|----|
| /FFTBinTopic0 | array di valori float in forma binaria, <br> _little-endian_ | due messaggi consecutivi da 4096 byte ciascuno [^1] | contiene i 2048 moduli dello spettro dell'accelerazione sul canale 0 |
| /FFTBinTopic1 | array di valori float in forma binaria, <br> _little-endian_ | due messaggi consecutivi da 4096 byte ciascuno [^1] | contiene i 2048 moduli dello spettro dell'accelerazione sul canale 1 |
| /RTD1BinTopic | stringa |  | temperatura in °C rilevata dalla RTD1 |
| /RTD2BinTopic | stringa |  | temperatura in °C rilevata dalla RTD2 |
| /RTD1FaultTopic | stringa |  | messaggio diagnostico dal modulo RTD1 <ul><li>Ok</li><li>RTD High Threshold</li><li>RTD Low Threshold</li><li>REFIN- > 0.85 x Bias</li><li>REFIN- < 0.85 x Bias - FORCE- open</li><li>RTDIN- < 0.85 x Bias - FORCE- open</li><li>Under/Over voltage</li></ul> |
| /RTD2FaultTopic | stringa |  | messaggio diagnostico dal modulo RTD2 <ul><li>Ok</li><li>RTD High Threshold</li><li>RTD Low Threshold</li><li>REFIN- > 0.85 x Bias</li><li>REFIN- < 0.85 x Bias - FORCE- open</li><li>RTDIN- < 0.85 x Bias - FORCE- open</li><li>Under/Over voltage</li></ul> |
| /CountAdcTopic | stringa |   | numero di campionamenti acquisiti sui canali coppia/velocità |
| /TorqueSpeedTopic | array di valori float in forma binaria, <br> _little-endian_ | un messaggio con 4 x CountAdcTopic elementi | contiene i campioni dei due canali di coppia e dei due canali di velocità, nel formato seguente <br> \[campioni coppia 1\]\[campioni coppia 2\]\[campioni velocità 1\]\[campioni velocità 2\]  |
| /pgaGetGain | stringa  |   | valore attuale del guadagno impostato sul PGA  |

[^1]:  il primo elemento del primo messaggio viene cambiato di segno (sarà sempre negativo) per consentire a chi riceve di discriminare il primo messaggio dal secondo messaggio e ricostruire l'ordine dei 4096+4096 byte, altrimenti lo spettro delle ampiezze potrebbe risultare traslato di metà banda passante.

---

## FUNZIONE SETUP()

Nella funzione `setup()` è importante rispettare la sequenza di operazioni:

1. configurare tutti i dispositivi hardware
2. assegnare i valori predefiniti a variabili/oggetti della applicazione
3. creare eventuali task RTOS
4. configurare il client MQTT con `configMqttClient(mqttServer, mqttPort, mqttUser, mqttPassword);`
5. avviare il sotto sistema WiFi con `initWiFi_STA();`
