# 🔴 RAPPORTO ANALISI - PROGETTO ESP32 ITEM_2324

**Data:** 5 Giugno 2026  
**Progetto:** ESP32 PlatformIO - Acquisizione ADS1256 + FFT + MQTT + WiFi  
**Analisi:** Completa - Bug, Race Conditions, Memory Leaks, Punti Critici  

---

## 📊 INDICE

1. [BUGS CRITICI](#bugs-critici-priorità-alta)
2. [PROBLEMI DI CONCORRENZA](#problemi-di-concorrenza-race-conditions)
3. [GESTIONE MEMORIA](#gestione-memoria-leaks-e-overflows)
4. [PUNTI CRITICI](#punti-critici-potenziali-problemi)
5. [RECOMMENDATIONS](#recommendations)

---

## 🔴 BUGS CRITICI (Priorità Alta)

### 1. **XHIGHERPRIORITYTASKWOKEN NON INIZIALIZZATO**
**File:** `src/APPLICATION/ISR/isr.cpp` (linea 8-10)  
**Severità:** 🔴 CRITICA  
**Tipo:** Comportamento Non Prevedibile (UB)

```cpp
void IRAM_ATTR ISR_DRDY()
{
  BaseType_t xHigherPriorityTaskWoken;  // ❌ NON INIZIALIZZATO!
  
  if (countData < FFT_SIZE)
  {
    xQueueSendFromISR(xQueueADS1256Sample, (void *)&countData, &xHigherPriorityTaskWoken);
    // ...
    if (xHigherPriorityTaskWoken)  // ❌ Lettura di valore casuale dalla memoria
    {
      portYIELD_FROM_ISR();
    }
  }
}
```

**Problema:**
- `xHigherPriorityTaskWoken` deve essere inizializzato a `pdFALSE` prima dell'uso
- Se non inizializzato, contiene un valore casuale da stack
- Può causare yield casuale, crash, o comportamento non deterministico
- FreeRTOS **richiede** questa inizializzazione

**Fix:**
```cpp
BaseType_t xHigherPriorityTaskWoken = pdFALSE;  // ✅ Inizializzare sempre
```

---

### 2. **RACE CONDITION - ACCESSO A countData SENZA SINCRONIZZAZIONE**
**File:** `src/APPLICATION/ISR/isr.cpp` + `src/APPLICATION/FSM/fsm.cpp`  
**Severità:** 🔴 CRITICA  
**Tipo:** Data Race

**ISR (isr.cpp, linea 10):**
```cpp
if (countData < FFT_SIZE)
{
  // ...
  countData++;  // ❌ Incremento senza protezione
}
```

**FSM (fsm.cpp, linea ~150):**
```cpp
case StartADC:
  // ...
  countData = 0;  // ❌ Reset senza protezione
```

**Problema:**
- `countData` è accesso dall'ISR (frequenza ~7500 Hz = ogni 133µs)
- La FSM accede in concorrenza senza mutex
- Possibile lettura di valore inconsistente durante reset
- Può causare perdita di campioni o crash

**Impatto:**
- Campioni persi
- Overflow della queue
- FSM in stato inconsistente

**Fix:**
```cpp
// In isr.cpp
if (xSemaphoreTakeFromISR(countDataSem, &xHigherPriorityTaskWoken))
{
  if (countData < FFT_SIZE)
  {
    countData++;
    xQueueSendFromISR(..., &xHigherPriorityTaskWoken);
  }
  xSemaphoreGiveFromISR(countDataSem, &xHigherPriorityTaskWoken);
}
```

---

### 3. **detachInterrupt LOGIC ERROR - INTERRUPT HANDLER INCONSISTENCY**
**File:** `src/APPLICATION/ISR/isr.cpp` (linea 14-18)  
**Severità:** 🔴 CRITICA  
**Tipo:** Logic Error

```cpp
void IRAM_ATTR ISR_DRDY()
{
  if (countData < FFT_SIZE)
  {
    // ... acquisisce campioni
    countData++;
  }
  else
  {
    detachInterrupt(nDRDY);  // ❌ Stacca interrupt
    countData = 0;           // ❌ Reset DOPO detach
  }
}
```

**Problema:**
- L'interrupt viene staccato quando `countData >= FFT_SIZE`
- `countData` viene resettato a 0, ma in quel momento l'interrupt non è più attivo
- Nel prossimo ciclo della FSM quando richiama `attachInterrupt(nDRDY, ISR_DRDY, FALLING)`, ma:
  - Se l'ISR viene chiamata prima che il contatore sia resettato, il comportamento è indefinito
  - **Race condition:** tra il detach e il reset di countData

**Scenario Problematico:**
1. FSM -> Sampling fase, `countData` raggiunge `FFT_SIZE`
2. ISR chiama `detachInterrupt(nDRDY)` e resetta `countData = 0`
3. FSM esegue `countData = 0` di nuovo in `StartADC` -> race!
4. Successivo `attachInterrupt()` - ma stato inconsistente

**Fix:**
```cpp
// Sincronizzare correttamente
if (countData >= FFT_SIZE)
{
  detachInterrupt(nDRDY);
  // Flag per FSM di resettare in modo atomico
}
// Reset dalla FSM con protezione
```

---

### 4. **CODA ADS1256 SIZE MISMATCH - OVERFLOW GARANTITO**
**File:** `src/APPLICATION/ADS1256/ADS1256Ext.h` (linea 5)  
**Severità:** 🔴 CRITICA  
**Tipo:** Buffer Overflow

```cpp
#define ADS1256QueueSize 20  // ❌ Dimensione queue
```

**Costanti (dati.h):**
```cpp
#define FFT_SIZE (1<<FFT_N)  // 2^12 = 4096 campioni
#define FSAMPLE 7500.0       // 7500 Hz
```

**Calcolo:**
- Ogni ciclo ISR produce 1 campione che va in coda
- FFT_SIZE = 4096 campioni necessari
- Queue size = 20 elementi
- **Queue overflow garantito dopo 20 campioni**

**Impatto:**
- Perdita di 4076 campioni su 4096 per ogni FFT!
- Campioni sovrapposti/scartati
- Corrupted FFT output

**Scenario:**
1. ISR inizia a scrivere in coda (7500 Hz = molto veloce)
2. FSM legge con timeout 5ms - non abbastanza veloce!
3. Queue piena dopo ~2.7ms
4. Campioni scartati silenziosamente

---

### 5. **MEMORY ALLOCATION WITHOUT NULL CHECK**
**File:** `src/APPLICATION/FFT/FFT.cpp` (linea 18-40)  
**Severità:** 🔴 CRITICA  
**Tipo:** Memory Leak / Crash

```cpp
fft_config_t *fft_init(int size, fft_type_t type, fft_direction_t direction, 
                       float *input, float *output)
{
  fft_config_t *config = (fft_config_t *)malloc(sizeof(fft_config_t));
  // ❌ NO NULL CHECK!
  
  // Allocate twiddle factors
  config->twiddle_factors = (float *)malloc(2 * config->size * sizeof(float));
  // ❌ Potrebbe fallire, config->twiddle_factors = NULL, usato poi
  
  // Allocate input
  if (input != NULL)
    config->input = input;
  else
  {
    config->input = (float *)malloc(config->size * sizeof(float));
    // ❌ NO NULL CHECK dopo malloc
    config->flags |= FFT_OWN_INPUT_MEM;
  }
  
  // Similarly for output...
}
```

**Calcoli Memoria:**
- `config`: 48 bytes
- `twiddle_factors`: 2 × 4096 × 4 = 32,768 bytes
- `input`: 4096 × 4 = 16,384 bytes
- `output`: 4096 × 4 = 16,384 bytes
- **Totale: ~65.5 KB per FFT**

**Problema:**
- Se malloc fallisce, ritorna NULL
- Il codice accede direttamente: `config->flags |= ...` → **Crash!**
- Nessun errore handling

**Impatto:**
- Crash ESP32 durante l'allocazione FFT
- Nessun recovery possibile

**Fix:**
```cpp
config = (fft_config_t *)malloc(sizeof(fft_config_t));
if (config == NULL) return NULL;

config->twiddle_factors = (float *)malloc(2 * config->size * sizeof(float));
if (config->twiddle_factors == NULL) 
{
  free(config);
  return NULL;
}
// ... etc
```

---

### 6. **MALLOC WITHOUT NULL CHECK IN parseMessage**
**File:** `src/MQTT/custom/parseMessage.cpp` (linea 10-11)  
**Severità:** 🟠 ALTA  
**Tipo:** Potential Crash

```cpp
void parseMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, 
                  size_t len, size_t index, size_t total)
{
  char *data = (char *)malloc((len + 1) * sizeof(char));
  // ❌ NO NULL CHECK
  
  strncpy(data, payload, len);  // ❌ Crash se data == NULL
  data[len] = '\0';
  // ...
  free(data);  // OK ma data potrebbe essere NULL (double free risk)
}
```

**Scenario:**
- Se MQTT riceve messaggio grande, malloc potrebbe fallire
- `strncpy` scrive a NULL → segmentation fault
- ESP32 reboot

---

### 7. **SPI TRANSACTION LEAKS - endTransaction MISSING**
**File:** `src/APPLICATION/MCP3204/mcp3204.cpp` (linea 31-42)  
**Severità:** 🔴 CRITICA  
**Tipo:** Resource Leak

```cpp
uint16_t mcp3204_getRaw(SPIClass &hwspi, uint8_t cs, uint8_t channel)
{
  uint16_t result;
  // ...
  hwspi.begin();
  hwspi.beginTransaction(SPISettings(MCP3204_SPI_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(cs, LOW);
  hwspi.transfer(command1);
  result = hwspi.transfer16(command2);
  digitalWrite(cs, HIGH);
  hwspi.endTransaction();  // ✅ OK
  return( result & 0x0fff);
}
```

**Ma in `StartADC` (fsm.cpp, linea ~85):**
```cpp
vspi.beginTransaction(SPISettings(MCP3204_SPI_CLOCK, MSBFIRST, SPI_MODE0));
// ... La transazione rimane APERTA per l'intero ciclo!
// Non c'è vspi.endTransaction() corrispondente
```

**Problema:**
- SPI transaction iniziata nel `StartADC` rimane aperta
- Viene chiusa solo quando FFT finisce (dopo ~600ms)
- Nel frattempo:
  - RTD, MAX31865 non possono usare VSPI
  - Possibile deadlock se altri task accedono VSPI
  - Bus bloccato

**Impatto:**
- Timeout SPI
- RTD temperature reading fallisce
- Starvation di altri task

---

### 8. **BLOCKING DELAYS IN MQTT PUBLISH - NO TIMEOUT**
**File:** `src/APPLICATION/TASK/publish_task.cpp` (linea 35-50)  
**Severità:** 🟠 ALTA  
**Tipo:** Infinite Blocking

```cpp
for (j = 0; j < BLOCKS_FLOAT; j++)
{
  do
  {
    if (MCP6S26_publish_channel_index == 0)
    {
      res = mqttClient.publish(...);
    }
    delay(25);  // ❌ Blocking delay
  } while (res == 0);  // ❌ Infinite loop! No timeout!
}
```

**Problema:**
- Se `mqttClient.publish()` fallisce (buffer pieno, disconnesso, etc.)
- Loop non ha mai fine!
- Task rimane bloccato indefinitamente con `delay(25)`
- Watchdog timer potrebbe triggerare (default 5s su ESP32)

**Conseguenze:**
- Reboot ESP32
- Perdita di altre acquisizioni

---

### 9. **UNINITIALIZED SPI VARIABLE IN MCP3204_getRaw**
**File:** `src/APPLICATION/MCP3204/mcp3204.cpp` (linea 31-42)  
**Severità:** 🟠 MEDIA  
**Tipo:** Logic Error

```cpp
uint16_t mcp3204_getRaw(SPIClass &hwspi, uint8_t cs, uint8_t channel)
{
  uint16_t result;
  // ❌ result NON INIZIALIZZATO
  
  uint8_t command1 = 0b00000110;
  uint16_t command2 = (channel & 0x03) << 14;
  
  // ... (però poi viene sempre assegnata via transfer16)
  result = hwspi.transfer16(command2);  // ✅ OK - sempre assegnata
  
  return( result & 0x0fff);
}
```

**Non è un problema critico** perché `result` viene sempre assegnata prima dell'uso, ma è cattiva pratica.

---

## 🟡 PROBLEMI DI CONCORRENZA (Race Conditions)

### 10. **UNSYNCHRONIZED GLOBAL STATE VARIABLES**
**File:** `src/APPLICATION/FSM/fsm.cpp` (linee varie)  
**Severità:** 🔴 CRITICA  
**Tipo:** Data Race

Variabili accesse da molteplici task **senza mutex/semaforo:**

```cpp
// FSM
volatile tStati _stato = WaitTrigger;              // ❌ Race
volatile bool dataReady = false;                    // ❌ Race
tMode triggered = Stop;                             // ❌ Race (setter da MQTT, reader da FSM)

// TASK
uint8_t MCP6S26_current_channel_index = 0;          // ❌ Race (ISR→FSM→Task)
uint8_t MCP6S26_publish_channel_index = 0;          // ❌ Race
```

**Accessi Concorrenti:**

| Variabile | Accesso1 | Accesso2 | Accesso3 |
|-----------|----------|----------|----------|
| `_stato` | FSM writes | publishFFT reads | sampleMCP3204 reads |
| `triggered` | MQTT callback writes | FSM reads | - |
| `dataReady` | FSM writes | publishFFT reads | - |
| `MCP6S26_current_channel_index` | ISR increments (?) | FSM reads/writes | publishFFT reads |

**Scenario Race Condition:**
```
Timer 1ms: MQTT riceve "trigger"=1 → set triggered=OneShot
Timer 1.5ms: FSM legge triggered → ma read è torn (16-bit su ARM32)
Timer 2ms: ISR aggiorna MCP6S26_current_channel_index
```

**Impatto:**
- Corrupted state
- Incompatible acquisition mode
- Crash

---

### 11. **DATAREADY VOLATILE BOOLEAN RACE**
**File:** `src/APPLICATION/FSM/fsm.cpp` (linea 110, 240)  
**Severità:** 🟡 MEDIA  
**Tipo:** Benign Race (ma potenzialmente problematico)

```cpp
case Sampling:
  // ...
  if (dataReady == true)  // ❌ Race: publishFFT potrebbe stare leggendo
  {
    dataReady = false;
    _stato = Compute;
  }
```

**Problema:**
- `publishFFT` aspetta il notify
- Mentre FSM scrive `dataReady = false`
- Possibile lost update

---

### 12. **MQTT RECONNECT TIMER WITHOUT SYNCHRONIZATION**
**File:** `src/MQTT/src/mqtt_setTimersRTOS.cpp` (linea 5)  
**Severità:** 🟡 MEDIA  
**Tipo:** Potential Null Pointer

```cpp
void setTimersRTOS(uint16_t timeout_ms)
{
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(timeout_ms), 
                                     pdFALSE, (void *)0, 
                                     reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  // ❌ NO NULL CHECK se xTimerCreate fallisce!
}
```

**Usato poi in:**
```cpp
void startTimersRTOS()
{
  xTimerStart(mqttReconnectTimer, 0);  // ❌ Crash se NULL
}
```

---

## 🔴 GESTIONE MEMORIA (Leaks e Overflows)

### 13. **MEMORY LEAK IN ADS1256 FFT PLAN**
**File:** `src/APPLICATION/FSM/fsm.cpp` (linea ~60)  
**Severità:** 🟡 MEDIA  
**Tipo:** Memory Leak (non critico, leak una sola volta)

```cpp
case StartADC:
  if (real_fft_plan == NULL)
  {
    real_fft_plan = fft_init(FFT_SIZE, FFT_REAL, FFT_FORWARD, NULL, NULL);
    // ✅ OK - allocato una sola volta
    pInput = real_fft_plan->input;
  }
  // ❌ real_fft_plan NEVER freed! Ma è init una sola volta per run, quindi minimale leak
```

**Impatto:** Minimale - viene allocato solo 1-2 volte in tutta la sessione. Se il programma ruotasse tra FreeRun → Stop → FreeRun mille volte, accumulererebbe leak.

---

### 14. **POTENTIAL HEAP FRAGMENTATION**
**Severità:** 🟡 MEDIA  
**Tipo:** Fragmentation Risk

**Allocazioni:**
- FFT init: ~65 KB (una volta)
- MQTT topics: Multiple `malloc()` per ogni topic
- Messaggi temporanei in parseMessage: (len + 1) bytes per ogni messaggio

**Con ESP32 heap ~300 KB:**
- Dopo ~50 diversi MQTT topic → ~5KB di overhead
- FFT fragmentation

**Rischio:**
- Heap non contiguo
- Allocation failure
- No recovery

---

### 15. **BUFFER OVERFLOW IN SPRINTF**
**File:** `src/APPLICATION/TASK/publish_task.cpp` (linea 68)  
**Severità:** 🟡 MEDIA  
**Tipo:** Potential Buffer Overflow

```cpp
float temp = 123.456;  // Example
char s[10];
sprintf(s, "%.3f", temp);  // "123.456\0" = 8 chars, OK
                           // ma "-123.456\0" = 9 chars
                           // e "9999.999\0" = 9 chars
                           // ❌ Borderline! Max 9 chars needed
```

**Scenario Crash:**
- Se RTD ritorna valore negativo vicino a max float
- Buffer size 10 potrebbe essere insufficiente

**Safe code:**
```cpp
char s[20];  // Margin di sicurezza
```

---

## 🟠 PUNTI CRITICI (Potenziali Problemi)

### 16. **POTENTIAL DEADLOCK - portMAX_DELAY USAGE**
**File:** `src/APPLICATION/TASK/publish_task.cpp` (linea 20)  
**Severità:** 🟠 MEDIA  
**Tipo:** Potential Deadlock

```cpp
void publishFFT(void *pvParameters)
{
  while (1)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // ❌ Indefinite wait!
    // Se notify non arriva mai → task bloccato per sempre
  }
}
```

**Scenario:**
1. `processTaskHandle` è NULL
2. O `xTaskNotifyGive(processTaskHandle)` non viene mai chiamato
3. Task rimane bloccato

**Impatto:** Risorsa inutilizzata, ma non critica poiché task riceve notify dalla FSM.

---

### 17. **INFINITE LOOP IN MCP3204 SAMPLING**
**File:** `src/APPLICATION/TASK/sampleMCP3204_task.cpp` (linea 11)  
**Severità:** 🟡 MEDIA  
**Tipo:** Potential Deadlock

```cpp
void sampleMCP3204(void *pvParameters)
{
  while (1)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // Aspetta notify
    
    if (mcp3204_BufferAvailable() > 0)
    {
      mcp3204_getAllVoltage(vspi, CS_MCP3204, &mcp3204_dati);
      mcp3204_Push(&mcp3204_dati);
    }
  }
}
```

**Problema:**
- Se `mcp3204_BufferAvailable() == 0`, non fa nulla e rimane in loop
- La notifica viene consumata ma nessun dato viene acquisito

---

### 18. **SPI CLOCK SPEED MISMATCH**
**File:** Vari (MCP3204, RTD, ADS1256)  
**Severità:** 🟡 MEDIA  
**Tipo:** Timing Issue

```cpp
// ADS1256 (isr.cpp/ADS1256Ext.cpp)
adc.init(hspi, nCS, nDRDY, nPDWN, 1900000);  // 1.9 MHz

// MCP3204 (mcp3204.cpp)
#define MCP3204_SPI_CLOCK 1250000  // 1.25 MHz

// MAX31865 (rtd_MAX31865.cpp)
// Non specificato, assume default
```

**Problema:**
- SPI bus multiplexed su 3+ dispositivi
- Freq diverse possono causare timing issues
- Nessun sincronizzazione tra i change di frequenza

---

### 19. **WIFI_BLOCK DURANTE SETUP**
**File:** `src/WIFI/src/wifi_init_STA.cpp` (linea 12-18)  
**Severità:** 🟠 MEDIA  
**Tipo:** Blocking Setup

```cpp
void initWiFi_STA()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println(F("Connecting to WiFi "));

  while (WiFi.status() != WL_CONNECTED)  // ❌ BLOCKING
  {
    Serial.print('.');
    delay(1000);  // Potrebbe bloccarsi 30+ secondi
  }
}
```

**Problema:**
- Nessuna task creata durante il WiFi connect
- Se WiFi non disponibile, setup() si blocca per minutes
- ADS1256 non acquisisce durante questo tempo

---

### 20. **QUEUE SIZE CALCULATIONS - POTENTIAL OVERFLOW**
**File:** `src/APPLICATION/CONSTANTS/dati.h`  
**Severità:** 🟡 MEDIA  
**Tipo:** Math Error

```cpp
#define MCP3204_NUMBER_OF_SAMPLES_PER_CHANNEL ( FFT_SIZE * 1000 / ((uint32_t)FSAMPLE) / MCP3204_PERIOD )
// = 4096 * 1000 / 7500 / 10
// = 4,096,000 / 75,000
// = 54.613... → 54 (integer division!)

#define MCP3204_BUFFER_SIZE (MCP3204_NUMBER_OF_SAMPLES_PER_CHANNEL * MCP3204_NUMBER_OF_CHANNELS)
// = 54 * 4 = 216 elementi di float
// = 216 * 4 = 864 bytes
```

**Problema:**
- Il numero non è esatto (54.6 → 54)
- Timing skew di ~0.6 campioni/ciclo
- Accumulo di clock drift nel tempo

---

### 21. **MISSING INTERRUPT PROTECTION - aux_din_status**
**File:** `src/APPLICATION/FSM/fsm.cpp` (linea ~235)  
**Severità:** 🟠 MEDIA  
**Tipo:** Race Condition

```cpp
// Compute state
aux_din_status = !digitalRead(AUX_DIN);  // Legge da GPIO

// Poi pubblicato in publish_task
if (aux_din_status)  // Altro task legge
{
  // publish
}
```

**Problema:**
- `aux_din_status` può cambiare mentre viene letto
- Non c'è sincronizzazione
- Ma è solo 1 bit, quindi rischio minore

---

### 22. **NO STACK OVERFLOW PROTECTION**
**Severità:** 🟡 MEDIA  
**Tipo:** Runtime Risk

**Task Stack Sizes (probabili, non configurati esplicitamente):**
- `processTaskHandle`: ~2-4 KB (FSM + FFT operations)
- `publishTaskHandle`: ~2-3 KB (MQTT publish)
- `sampleMCP3204TaskHandle`: ~1-2 KB

**ESP32 default stack:**
- Potrebbe essere insufficiente per FFT + MQTT operations
- Stack overflow → crash silenzioso

---

### 23. **NO MQTT QOS CONFIGURATION**
**File:** `src/MQTT/src/mqtt_onMqttConnect.cpp` (linea 20)  
**Severità:** 🟡 BASSA  
**Tipo:** Design Issue

```cpp
ptr->pktId = mqttClient.subscribe(ptr->topic, ptr->qos);
```

Non è chiaro quali `qos` viene usato. Se `qos` è undefined, comportamento imprevedibile.

---

### 24. **MISSING CONFIGURATION VALIDATION**
**File:** `main.cpp`  
**Severità:** 🟠 MEDIA  
**Tipo:** Setup Validation

**Manca:**
- Verifica che `FFT_SIZE` sia power-of-2 ✓ (fatto in fft_init)
- Verifica che `FSAMPLE` e `MCP3204_PERIOD` siano > 0
- Verifica credenziali WiFi/MQTT non vuote

---

## ✅ RECOMMENDATIONS

### Priority 1: Eseguire IMMEDIATAMENTE

#### 1. **Fix ISR xHigherPriorityTaskWoken**
```cpp
// isr.cpp
void IRAM_ATTR ISR_DRDY()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;  // ✅ INIT
  
  if (countData < FFT_SIZE)
  {
    xQueueSendFromISR(xQueueADS1256Sample, (void *)&countData, &xHigherPriorityTaskWoken);
    countData++;
    
    if (xHigherPriorityTaskWoken)
    {
      portYIELD_FROM_ISR();
    }
  }
}
```

#### 2. **Fix Queue Size Mismatch**
```cpp
// ADS1256Ext.h
#define ADS1256QueueSize 4096  // ✅ = FFT_SIZE
```

#### 3. **Sincronizzare countData**
```cpp
// Oppure usare SemaphoreHandle_t countDataSem inizializzato in main
// Oppure usare atomic operations: taskENTER_CRITICAL / taskEXIT_CRITICAL
```

#### 4. **Fix FFT malloc NULL checks**
Aggiungere controllo NULL dopo ogni malloc in `FFT.cpp`.

#### 5. **Fix MQTT publish infinite loop**
```cpp
// publish_task.cpp - Aggiungere timeout
uint32_t start = millis();
do
{
  res = mqttClient.publish(...);
  if (millis() - start > 5000) break;  // ✅ 5s timeout
  delay(25);
} while (res == 0);
```

---

### Priority 2: Correggere PRIMA del deployment

#### 6. **Aggiungi Mutex su variabili globali**
```cpp
// fsm.h
extern SemaphoreHandle_t fsmStateSem;  // Protegge _stato, dataReady, triggered
```

#### 7. **Fix SPI Transaction**
```cpp
// fsm.cpp - Aggiungere endTransaction nel Sampling state quando finito
case Sampling:
  // ...
  if (sampleCounter >= FFT_SIZE)
  {
    vspi.endTransaction();  // ✅ ADD
    adc.standby();
  }
```

#### 8. **Fix detachInterrupt Logic**
```cpp
// Usare flag invece di countData
volatile bool sampling_complete = false;

void IRAM_ATTR ISR_DRDY()
{
  if (!sampling_complete && countData < FFT_SIZE)
  {
    xQueueSendFromISR(xQueueADS1256Sample, (void *)&countData, &xHigherPriorityTaskWoken);
    countData++;
  }
  else if (countData >= FFT_SIZE)
  {
    sampling_complete = true;
  }
}

// FSM
case StartADC:
  countData = 0;
  sampling_complete = false;
  attachInterrupt(nDRDY, ISR_DRDY, FALLING);
  break;
```

#### 9. **Aggiungere NULL checks dopo malloc**
```cpp
// parseMessage.cpp
char *data = (char *)malloc((len + 1) * sizeof(char));
if (data == NULL)
{
  Serial.println("ERROR: malloc failed in parseMessage");
  return;
}
```

#### 10. **Fix mqtt timer NULL check**
```cpp
// mqtt_setTimersRTOS.cpp
void setTimersRTOS(uint16_t timeout_ms)
{
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(timeout_ms), 
                                     pdFALSE, (void *)0, 
                                     reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  if (mqttReconnectTimer == NULL)
  {
    Serial.println("ERROR: xTimerCreate failed");
  }
}
```

---

### Priority 3: Miglioramenti a lungo termine

#### 11. **Aggiungere proper synchronization primitives**
- Usare `SemaphoreHandle_t` per variabili condivise
- Oppure usare `atomic` operations

#### 12. **Increase heap size**
```cpp
// platformio.ini
build_flags = -DCONFIG_HEAP_MEMORY_MONITOR=1
             -DCONFIG_SPIRAM_SUPPORT=1
```

#### 13. **Implement watchdog monitor**
Aggiungere task monitor per verificare se altri task non rispondono.

#### 14. **Add error handling**
- Try-catch per operazioni MQTT
- Fallback graceful se acquisizione fallisce

#### 15. **Optimize FFT allocation**
Usare allocazione pre-buffer in heap esterno (SPIRAM) se disponibile.

---

## 📋 CHECKLIST FIX

- [ ] Inizializzare `xHigherPriorityTaskWoken`
- [ ] Aumentare `ADS1256QueueSize` a 4096
- [ ] Aggiungere NULL check dopo malloc (FFT, MQTT, etc.)
- [ ] Aggiungere timeout ai publish_task loop
- [ ] Aggiungere mutex su variabili globali (_stato, triggered, dataReady)
- [ ] Fix SPI transaction endTransaction
- [ ] Fix detachInterrupt logic
- [ ] Test con heap monitor
- [ ] Aggiungere logging per debug race conditions

---

## 🧪 TESTING RECOMMENDATIONS

1. **Stress test:**
   - Ruotare tra Stop→OneShot→FreeRun 1000 volte
   - Verificare no memory leaks

2. **MQTT stress:**
   - Inviare messaggi trigger ogni 100ms
   - Verificare no infinite loops

3. **ISR timing:**
   - Misurare tempo ISR con GPIO toggle
   - Verificare FFT non interrotto

4. **Heap monitoring:**
   - `Serial.printf("Free heap: %d\n", ESP.getFreeHeap());`
   - Verificare no fragmentazione

---

**Fine Rapporto**  
*Analisi completata: 5 Giugno 2026*
