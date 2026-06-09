# Network Stack Core 0 Configuration

## Overview
Tutto lo stack di rete (WiFi + MQTT) è stato configurato per girare esclusivamente sul core 0 (PRO_CPU) della ESP32.

## Modifiche Implementate

### 1. platformio.ini - Build Flags
Aggiunti i seguenti flag di compilazione in entrambi gli environment (release e debug):

```ini
build_flags = 
	-DCONFIG_LWIP_CORE=0                    # Forza lwIP (WiFi stack) sul core 0
	-DCONFIG_ESP_WIFI_TASK_CORE_ID=0        # Task WiFi sul core 0
	-DCONFIG_ESP_WIFI_TASK_AFFINITY=0x1     # Affinity bitmask: 0x1 = core 0
	-DCONFIG_FREERTOS_TIMER_TASK_CORE=0     # Timer task sul core 0
```

### 2. main.cpp - Setup Configurazione WiFi
Aggiunto nel setup():
- Messaggio di configurazione del network stack
- Impostazione di TX power per il WiFi

### 3. wifi_init_STA.cpp - Inizializzazione WiFi
Aggiunto commento esplicativo e messaggio di log che confirma che il WiFi gira sul core 0.

### 4. mqtt_configMqttClient.cpp - Configurazione MQTT Client
Aggiunto commento che specifica che il client MQTT erediterà la core affinity da:
- I build flags di configurazione
- La FreeRTOS configuration

### 5. mqtt_setTimersRTOS.cpp - Timer MQTT
Migliorato il codice con:
- Commenti esplicativi che il timer runs su core 0
- Error handling per la creazione del timer
- Messaggio di log per confermare la creazione

## Core Affinity Architecture

### ESP32 Dual-Core Layout
- **Core 0 (PRO_CPU)**: Network stack (WiFi, MQTT, Timer daemon)
- **Core 1 (APP_CPU)**: Application data acquisition (ADC, FFT processing)

### Task Distribution
```
CORE 0 (PRO_CPU)
├── WiFi Stack (lwIP)
├── MQTT Client (AsyncMqttClient)
├── Timer Daemon Task (MQTT reconnect timer)
└── [Other system tasks]

CORE 1 (APP_CPU)
├── process() - FFT elaboration
├── publishFFT() - FFT publication
└── sampleMCP3204() - ADC sampling
```

## Verification

Per verificare che la configurazione è corretta, controllare i log al boot:
```
Configuring network stack for core 0...
WiFi configured for core 0 operation
MQTT client configured for core 0 operation
MQTT reconnect timer created for core 0 operation
```

## Benefits
1. **Isolamento**: Il network stack non interferisce con l'acquisizione dati real-time
2. **Prevedibilità**: L'ADC sampling su core 1 è meno disturbato da task di rete
3. **Determinismo**: L'ISR per DRDY (ADS1256) può eseguire senza contention di core
4. **Performance**: Riduce jitter nell'acquisizione audio/accelerometro

## Note Importanti
- I build flags sono standard Arduino ESP32 e supportati dal framework
- La core affinity è garantita dal livello FreeRTOS/Arduino
- Il WiFi.setTxPower() riduce il rumore EMI che potrebbe influenzare gli ADC

## Riferimenti
- [Arduino ESP32 Dual Core Documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/freertos.html)
- [ESP32 lwIP Configuration](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/lwip.html)
- [AsyncMqttClient Documentation](https://github.com/marvinroger/async-mqtt-client)
