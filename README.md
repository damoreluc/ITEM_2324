# WiFi Esempio 05: <br>template di client MQTT con connessione a rete WiFi

Il progetto è un modello base per realizzare un client MQTT. Va considerato come riferimento per progetti più articolati basati sul protocollo MQTT su ESP32.

Impiega la libreria `AsyncMqttClient` (link: https://github.com/OttoWinter/async-mqtt-client)

(vedi anche https://github.com/khoih-prog/AsyncMQTT_ESP32 con documentazione più completa)

L'esempio accetta quattro subscribed topics:

* `ESP32_base/yellowTopic`  per il comando del led giallo
* `ESP32_base/redTopic`  per il comando del led rosso
* `ESP32_base/blueTopic`  per il comando del led blu 
* `ESP32_base/input`    per la stampa di un messaggio di testo sulla console seriale

e pubblica sul topic:

* `ESP32_base/output`

un testo che rappresenta lo stato attuale di un pulsante.

---

## Cartella `\APPLICATION`

La cartella `\APPLICATION` è pensata per organizzare meglio il codice del progetto.<br>Inserire qui le funzioni ausiliarie specifiche per l'applicazione in sviluppo, ad esempio le callback di accensione e spegnimento dei led in funzione del contenuto del payload.

---

## CONFIGURAZIONE WIFI

La scheda ESP32 è configurata come __STATION__ in una rete WiFi:

* modificare il file `WIFI/credentials.h` con i propri `SSID` e `Password`

Per il controllo dello stato della connessione vengono adoperati gli eventi dell'oggetto Wifi.

Vengono gestite le situazioni di perdita di connessione mediante riconnessione automatica.

Il pin `23` viene impiegato come uscita digitale per indicare la connessione all'access point wiFi. Per configurare un'altra posizione del led, modificare il parametro `pinWiFiConnected` nel file `HWCONFIG\hwConfig.h`

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

__IMPORTANTE__: __NON__ includere più di un file di definizione del broker.

## DEFINIZIONE DEI SUBSCRIBED E DEI PUBLISHING TOPICS

In base all'applicazione da creare, vanno definiti i _topics_ a cui il client deve iscriversi per ricevere dati o comandi da remoto.

Il client MQTT deve possedere un nome __univoco__ sul broker. Nel file `MQTT\custom\mqtt_topics.h`
viene dichiarato il nome univoco, che è possibile personalizzare:

```C
  // MQTT client ID
  #define thisClient "ESP32_base"
```

Vanno poi definiti i topic sui quali il client pubblica dati verso il broker.

Allo scopo vengono impiegati due dizionari:

* per i _subscribed topics_ ("ingressi" per il client):  `subscribedTopics`
* per i _publishing topics_ ("uscite" del client):  `publishedTopics`

Entrambi i dizionari hanno la struttura:  `<chiave>, <valore>` entrambe le voci sono di tipo `String`.

* Con `<chiave>` si indica un nome semplice da assegnare al topic
* Il `<valore>` contiene il _persorso logico_ del topic.


La personalizzazione va svolta nel file `MQTT\custom\mqtt_topics.cpp`

* Per i topic in ingresso (_Subscribed topics_) modificare la funzione:

```C
  void compileSubTopics(Dictionary<String, String> &subTopics)
```

* Per i topic in uscita (_Published topics_) modificare la funzione:

```C
  void compilePubTopics(Dictionary<String, String> &pubTopics)
```

__Procedere nell'ordine:__

1. definire quali informazioni la scheda ESP32 dovrà ricevere dal broker e associare i subscribed topics, definendo un nome univoco (chiave) per ciascun topic e il suo percorso sul broker.<br>
Ad esempio: si vuole accendere/spegnere da remoto un led giallo collegato alla ESP32;
    * si definisce il _subscribed topic_ con nome (chiave) `"yellowOnOffTopic"`
    * con percorso  `"ESP32_base/yellowTopic"` 
    * da questo topic arriverà la stringa `"0"` per spegnere il led, o la stringa `"1"` per accenderlo.

2. in modo simile, definire quali informazioni la scheda ESP32 pubblicherà verso il broker e associare i publisher topics, definendo un nome univoco per ciascun topic e il suo percorso sul broker.<br>
Ad esempio: si vuole notificare in remoto che un pulsante è stato premuto o rilasciato;
    * si definisce il _published topic_ con nome `"outTopic"`
    * con percorso  `"ESP32_base/output"`
    * su questo topic la ESP32 invierà un messaggio sullo stato del pulsante

3. per i subscribed topics, nella funzione `compileSubTopics()` aggiungere il topic al dizionario dei subscribed topics mediante il comando:

```C
   // subscribed topic di comando del led giallo  
   subTopics.set("yellowOnOffTopic", thisClient "/yellowTopic");
```

4. ripetere il punto 3. per ciascun _subscribed topic_ richiesto dalla applicazione
5. per i _publishing topic_, nella funzione `compilePubTopics()` aggiungere il topic al dizionario dei published topics mediante il comando:

```C
   // topic di pubblicazione messaggi
   pubTopics.set("outTopic", thisClient "/output");
```

 6. ripetere il punto 5. per ciascun _published topic_ richiesto dalla applicazione.

All'avvio del client MQTT verranno automaticamente registrati sul broker i subscribed topics.

## PARSING DEI SUBSCRIBED TOPICS

Il client MQTT gestisce il traffico col broker in modo _asincrono_, non è necessario che il programmatore della applicazione si preoccupi delle fasi di ricezione o di trasmissione dei dati sui topics.

E' invece __responsabilità del programmatore__ decidere cosa fare quando viene ricevuto un messaggio su un subscribed topic.

Nel file     `MQTT\custom\parseMessage.cpp`

va personalizzata la funzione    `parseMessage()`

Rifacendosi al caso del led giallo illustrato più sopra, nella `parseMessage()` si andrà a:

1. verificare se si tratta del __topic__ descritto dalla chiave `"yellowOnOffTopic"` allora si passa il contenuto del payload alla funzione ausiliaria __programmata dallo sviluppatore__ che gestirà l'informazione:

```C
// operazioni da eseguire quando viene ricevuto un messaggio
// viene richiamata da mqtt_onMqttMessage()
void parseMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    // bonifica del payload
    // estrae solo i primi len caratteri del payload
    char data[len + 1];
    strncpy(data, payload, len);

    // print some information about the received message
    printRcvMsg(topic, payload, properties, len, index, total);

    // da personalizzare

    // comando del led giallo
    // è arrivato un messaggio da yellowOnOffTopic
    if (strcmp(topic, subscribedTopics.get("yellowOnOffTopic").c_str()) == 0)
    {
        // comanda on/off led giallo a partire dal payload
        driveOnOffYellow(data);
    }
}
```

2. nella funzione ausiliaria, se il __payload__ è la stringa `"0"` viene comandato lo spegnimento del led giallo altrimenti se il __payload__ è la stringa `"1"` viene comandata l'accensione del led giallo.

La struttura del codice corrispondente è:

```C
#include <APPLICATION/application.h>
#include <HWCONFIG/hwConfig.h>

// comanda on/off led giallo a partire dal payload
void driveOnOffYellow(char *data)
{
    if (strncmp(data, "0", 1) == 0)
    {
        digitalWrite(pinYellow, LOW);
        Serial.println("led giallo spento");
    }
    else if (strncmp(data, "1", 1) == 0)
    {
        digitalWrite(pinYellow, HIGH);
        Serial.println("led giallo acceso");
    }
}
```

__NOTA__: nelle operazioni di confronto tra stringhe del payload (trattate come _array di char_) è consigliato utilizzare la funzione di confronto `strncmp()` specificando esattamente il numero di caratteri da confrontare.

## PUBBLICAZIONE DI UN DATO SUI PUBLISHING TOPICS

Anche la pubblicazione di un dato dalla ESP32 verso il broker viene gestita in modalità asincrona dai layers della libreria MQTT.

Nella applicazione è sufficiente che il programmatore utilizzi il metodo `mqttClient.publish()` ogni qual volta desidera pubblicare una informazione su un particolare topic.

Il metodo `mqttClient.publish()` richiede i seguenti parametri:

* il topic sul quale pubblicare, ad esempio `"ESP32_base/output"`
* il livello di QOS (ad esempio 0)
* se il messaggio è di tipo retain o no
* il payload da trasmettere (ad esempio un array di char contenente un testo)
* la dimensione _in byte_ del payload

e ritorna il _packet ID_ (diverso da 0) del messaggio se è stato in grado di inserire il payload nella coda dei messaggi da pubblicare verso il broker, altrimenti restituisce il codice di errore 0.

Ad esempio, per pubblicare lo stato del pulsante, il programmatore può scrivere:

```C
  if (button.fell())
  {
    const char msgButton[] = "Pulsante premuto";
    Serial.println(msgButton);

    // pubblica sul topic outTopic
    if(mqttClient.connected()) {
      uint16_t res = 0;
      res = mqttClient.publish(publishedTopics.get("outTopic").c_str(),0,false, msgButton, strlen(msgButton));
    }
  }
```

Si osservi che il payload (l'informazione inviata sul canale definito dal topic) verrà trattata dai layers di basso livello come __array di byte__. Pertanto il payload può anche essere un `int`, un `float` o un tipo dati definito dall'utente, purché sia determinabile la sua dimensione in byte e che il tipo dati sia _riconoscibile_ e _gestibile_ da chi riceverà l'informazione dall'altro lato del broker.

---

## FUNZIONE SETUP()

Nella funzione `setup()` è importante rispettare la sequenza di operazioni:

1. configurare tutti i dispositivi hardware
2. assegnare i valori predefiniti a variabili/oggetti della applicazione
3. creare eventuali task RTOS
4. configurare il client MQTT con `configMqttClient(mqttServer, mqttPort, mqttUser, mqttPassword);`
5. avviare il sotto sistema WiFi con `initWiFi_STA();`
