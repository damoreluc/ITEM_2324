# FIX IMPLEMENTATION GUIDE - ITEM_2324 ESP32

## 🔧 Applicazione Patch per Risolvere I Bugs

Questo documento contiene il codice esatto da applicare per risolvere i bug critici.

---

## FIX #1: ISR xHigherPriorityTaskWoken Inizializzazione

**File:** `src/APPLICATION/ISR/isr.cpp`

**PRIMA (ERRATO):**
```cpp
void IRAM_ATTR ISR_DRDY()
{
  BaseType_t xHigherPriorityTaskWoken;  // ❌ Non inizializzato
```

**DOPO (CORRETTO):**
```cpp
void IRAM_ATTR ISR_DRDY()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;  // ✅ Inizializzato
```

---

## FIX #2: Queue Size Mismatch

**File:** `src/APPLICATION/ADS1256/ADS1256Ext.h`

**PRIMA (ERRATO):**
```cpp
#define ADS1256QueueSize 20  // ❌ FFT_SIZE è 4096!
```

**DOPO (CORRETTO):**
```cpp
// Opzione 1: Usa FFT_SIZE
#define ADS1256QueueSize FFT_SIZE  // ✅ Dinamico

// Opzione 2: Valore fisso sicuro
#define ADS1256QueueSize 4096  // ✅ Corrisponde a FFT_SIZE
```

---

## FIX #3: FFT Memory Allocation NULL Checks

**File:** `src/APPLICATION/FFT/FFT.cpp`

**PRIMA (ERRATO):**
```cpp
fft_config_t *fft_init(int size, fft_type_t type, fft_direction_t direction, 
                       float *input, float *output)
{
  fft_config_t *config = (fft_config_t *)malloc(sizeof(fft_config_t));
  // ❌ NO NULL CHECK

  config->twiddle_factors = (float *)malloc(2 * config->size * sizeof(float));
  // ❌ NO NULL CHECK - CRASH if malloc fails

  if (input != NULL)
    config->input = input;
  else
  {
    config->input = (float *)malloc(config->size * sizeof(float));
    // ❌ NO NULL CHECK
    config->flags |= FFT_OWN_INPUT_MEM;
  }

  if (config->input == NULL)  // This check is AFTER use - too late!
    return NULL;
  // ... similar for output
}
```

**DOPO (CORRETTO):**
```cpp
fft_config_t *fft_init(int size, fft_type_t type, fft_direction_t direction, 
                       float *input, float *output)
{
  int k, m;

  // ✅ Check allocation #1
  fft_config_t *config = (fft_config_t *)malloc(sizeof(fft_config_t));
  if (config == NULL) {
    Serial.println("ERROR: fft_init malloc config failed");
    return NULL;
  }

  // Check if the size is a power of two
  if ((size & (size - 1)) != 0) {
    free(config);
    Serial.println("ERROR: FFT size must be power of 2");
    return NULL;
  }

  config->flags = 0;
  config->type = type;
  config->direction = direction;
  config->size = size;

  // ✅ Check allocation #2
  config->twiddle_factors = (float *)malloc(2 * config->size * sizeof(float));
  if (config->twiddle_factors == NULL) {
    free(config);
    Serial.println("ERROR: fft_init malloc twiddle_factors failed");
    return NULL;
  }
  // ... rest of twiddle initialization
  config->flags |= FFT_OWN_TWIDDLE_MEM;

  // Allocate input buffer
  if (input != NULL) {
    config->input = input;
  } else {
    if (config->type == FFT_REAL) {
      config->input = (float *)malloc(config->size * sizeof(float));
    } else if (config->type == FFT_COMPLEX) {
      config->input = (float *)malloc(2 * config->size * sizeof(float));
    }
    
    // ✅ Check allocation #3
    if (config->input == NULL) {
      free(config->twiddle_factors);
      free(config);
      Serial.println("ERROR: fft_init malloc input failed");
      return NULL;
    }
    config->flags |= FFT_OWN_INPUT_MEM;
  }

  // Allocate output buffer
  if (output != NULL) {
    config->output = output;
  } else {
    if (config->type == FFT_REAL) {
      config->output = (float *)malloc(config->size * sizeof(float));
    } else if (config->type == FFT_COMPLEX) {
      config->output = (float *)malloc(2 * config->size * sizeof(float));
    }
    
    // ✅ Check allocation #4
    if (config->output == NULL) {
      if (config->flags & FFT_OWN_INPUT_MEM)
        free(config->input);
      free(config->twiddle_factors);
      free(config);
      Serial.println("ERROR: fft_init malloc output failed");
      return NULL;
    }
    config->flags |= FFT_OWN_OUTPUT_MEM;
  }

  return config;
}
```

---

## FIX #4: MQTT Publish Infinite Loop Timeout

**File:** `src/APPLICATION/TASK/publish_task.cpp`

**PRIMA (ERRATO):**
```cpp
for (j = 0; j < BLOCKS_FLOAT; j++)
{
  do
  {
    if (MCP6S26_publish_channel_index == 0)
    {
      res = mqttClient.publish(publishedTopics.get("outTopic0").c_str(), 0, false, 
                              (const char *)&(real_fft_plan->output[BLOCK_FLOAT_HALF_SIZE * j]), 
                              MQTT_MAX_SIZE_BYTE);
    }
    else if (MCP6S26_publish_channel_index == 1)
    {
      res = mqttClient.publish(publishedTopics.get("outTopic1").c_str(), 0, false, 
                              (const char *)&(real_fft_plan->output[BLOCK_FLOAT_HALF_SIZE * j]), 
                              MQTT_MAX_SIZE_BYTE);
    }

    delay(25);
  } while (res == 0);  // ❌ INFINITE LOOP - no timeout!
}
```

**DOPO (CORRETTO):**
```cpp
#define MQTT_PUBLISH_TIMEOUT_MS 5000  // ✅ Add timeout

for (j = 0; j < BLOCKS_FLOAT; j++)
{
  uint32_t publish_start = millis();
  bool publish_success = false;
  
  do
  {
    if (MCP6S26_publish_channel_index == 0)
    {
      res = mqttClient.publish(publishedTopics.get("outTopic0").c_str(), 0, false, 
                              (const char *)&(real_fft_plan->output[BLOCK_FLOAT_HALF_SIZE * j]), 
                              MQTT_MAX_SIZE_BYTE);
    }
    else if (MCP6S26_publish_channel_index == 1)
    {
      res = mqttClient.publish(publishedTopics.get("outTopic1").c_str(), 0, false, 
                              (const char *)&(real_fft_plan->output[BLOCK_FLOAT_HALF_SIZE * j]), 
                              MQTT_MAX_SIZE_BYTE);
    }

    if (res != 0) {
      publish_success = true;  // ✅ Success
      break;
    }
    
    // ✅ Check timeout
    if (millis() - publish_start > MQTT_PUBLISH_TIMEOUT_MS) {
      Serial.printf("MQTT publish timeout for block %d\n", j);
      break;
    }

    delay(25);
  } while (!publish_success);
}
```

---

## FIX #5: parseMessage malloc NULL Check

**File:** `src/MQTT/custom/parseMessage.cpp`

**PRIMA (ERRATO):**
```cpp
void parseMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, 
                  size_t len, size_t index, size_t total)
{
    // payload clean up
    char *data = (char *)malloc((len + 1) * sizeof(char));
    // ❌ NO NULL CHECK
    strncpy(data, payload, len);  // ❌ CRASH if data == NULL
    data[len] = '\0';

    printRcvMsg(topic, payload, properties, len, index, total);

    // MSF Acquisition Command
    if (strcmp(topic, subscribedTopics.get("triggerTopic").c_str()) == 0)
    {
        triggerFSM(data);
    }
    else if (strcmp(topic, subscribedTopics.get("pgaSetGainTopic").c_str()) == 0) 
    {
        setPGAgain(data);
    }  

    free(data);  // ❌ May be NULL
}
```

**DOPO (CORRETTO):**
```cpp
void parseMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, 
                  size_t len, size_t index, size_t total)
{
    // ✅ Check malloc
    char *data = (char *)malloc((len + 1) * sizeof(char));
    if (data == NULL)
    {
        Serial.println("ERROR: parseMessage malloc failed");
        return;  // ✅ Exit safely
    }
    
    strncpy(data, payload, len);
    data[len] = '\0';

    printRcvMsg(topic, payload, properties, len, index, total);

    // MSF Acquisition Command
    if (strcmp(topic, subscribedTopics.get("triggerTopic").c_str()) == 0)
    {
        triggerFSM(data);
    }
    else if (strcmp(topic, subscribedTopics.get("pgaSetGainTopic").c_str()) == 0) 
    {
        setPGAgain(data);
    }  

    free(data);  // ✅ Always freed (data is non-NULL here)
}
```

---

## FIX #6: SPI Transaction Missing endTransaction

**File:** `src/APPLICATION/FSM/fsm.cpp`

**PRIMA (ERRATO):**
```cpp
case StartADC:
    // ... setup code ...
    vspi.beginTransaction(SPISettings(MCP3204_SPI_CLOCK, MSBFIRST, SPI_MODE0));
    // ❌ Transaction opened but NEVER closed!
    // ... rest of StartADC ...
    _stato = Sampling;
    break;

case Sampling:
    // ... sampling code ...
    if (sampleCounter >= FFT_SIZE)
    {
        adc.standby();
        vspi.endTransaction();  // ✅ Only here, but too late!
        // ...
    }
    break;
```

**DOPO (CORRETTO):**
```cpp
case StartADC:
    // ... setup code ...
    
    if (getSensMode() == REAL_DATA)
    {
        vspi.beginTransaction(SPISettings(MCP3204_SPI_CLOCK, MSBFIRST, SPI_MODE0));
        
        pga0.channel = pga0_channels[MCP6S26_current_channel_index];
        mcp6s26_setChannel(vspi, CS_PGA0, pga0.channel);
        // ... rest of setup ...
    }
    
    _stato = Sampling;
    sampleCounter = 0;
    meanValue = 0.0;
    break;

case Sampling:
    if (getSensMode() == REAL_DATA)
    {
        // ... sampling code ...
        
        if (sampleCounter >= FFT_SIZE)
        {
            adc.standby();
            // ✅ End transaction when sampling is done
            vspi.endTransaction();
            
            vspi.beginTransaction(SPISettings(MCP3204_SPI_CLOCK, MSBFIRST, SPI_MODE0));
            
            sampleCounter = 0;
            countData = 0;
            dataReady = true;
        }
    }
    
    if (dataReady == true)
    {
        dataReady = false;
        _stato = Compute;
    }
    break;
```

---

## FIX #7: detachInterrupt Logic Fix

**File:** `src/APPLICATION/ISR/isr.cpp` + `src/APPLICATION/FSM/fsm.cpp`

**PRIMA (ERRATO):**
```cpp
// isr.cpp
void IRAM_ATTR ISR_DRDY()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (countData < FFT_SIZE)
  {
    xQueueSendFromISR(xQueueADS1256Sample, (void *)&countData, &xHigherPriorityTaskWoken);
    countData++;
  }
  else
  {
    detachInterrupt(nDRDY);  // ❌ Interrupt detached
    countData = 0;           // ❌ Reset happens AFTER detach
  }
}

// fsm.cpp StartADC
case StartADC:
    countData = 0;  // ❌ Race: countData might already be 0 from ISR
    attachInterrupt(nDRDY, ISR_DRDY, FALLING);
```

**DOPO (CORRETTO):**
```cpp
// ADS1256Ext.h - Add new flag
extern volatile bool sampling_complete;

// isr.cpp
void IRAM_ATTR ISR_DRDY()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  // ✅ Use flag instead of detachInterrupt
  if (!sampling_complete && countData < FFT_SIZE)
  {
    xQueueSendFromISR(xQueueADS1256Sample, (void *)&countData, &xHigherPriorityTaskWoken);
    countData++;

    if (xHigherPriorityTaskWoken)
    {
      portYIELD_FROM_ISR();
    }
  }
  else if (countData >= FFT_SIZE)
  {
    // ✅ Signal completion instead of detaching
    sampling_complete = true;
  }
}

// ADS1256Ext.cpp - Initialize flag
volatile bool sampling_complete = false;

// fsm.cpp
case StartADC:
    // ✅ Reset atomically
    countData = 0;
    sampling_complete = false;  // ✅ Reset flag
    
    if (getSensMode() == REAL_DATA)
    {
        // ... setup code ...
        attachInterrupt(nDRDY, ISR_DRDY, FALLING);
    }
    
    _stato = Sampling;
    sampleCounter = 0;
    meanValue = 0.0;
    break;
```

---

## FIX #8: MQTT Timer NULL Check

**File:** `src/MQTT/src/mqtt_setTimersRTOS.cpp`

**PRIMA (ERRATO):**
```cpp
void setTimersRTOS(uint16_t timeout_ms)
{
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(timeout_ms), 
                                     pdFALSE, (void *)0, 
                                     reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  // ❌ NO NULL CHECK
}
```

**DOPO (CORRETTO):**
```cpp
void setTimersRTOS(uint16_t timeout_ms)
{
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(timeout_ms), 
                                     pdFALSE, (void *)0, 
                                     reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  
  // ✅ Check for NULL
  if (mqttReconnectTimer == NULL)
  {
    Serial.println("ERROR: Failed to create MQTT reconnect timer");
    return;
  }
}

// Also check in startTimersRTOS
void startTimersRTOS()
{
  if (mqttReconnectTimer != NULL)  // ✅ Guard
  {
    xTimerStart(mqttReconnectTimer, 0);
  }
  else
  {
    Serial.println("ERROR: mqttReconnectTimer is NULL");
  }
}
```

---

## FIX #9: Add Synchronization for Global State

**File:** `src/APPLICATION/FSM/fsm.h`

**ADD:**
```cpp
#ifndef _FSM_H
#define _FSM_H

#include <Arduino.h>

// ✅ Add synchronization primitive
extern SemaphoreHandle_t fsmStateMutex;

// modalità di acquisizione
typedef enum
{
  Stop,
  OneShot,
  FreeRun
} tMode;

extern tMode triggered;

// ... rest of file ...
```

**File:** `src/APPLICATION/FSM/fsm.cpp`

**ADD:**
```cpp
// ✅ Create mutex in setup or module init
SemaphoreHandle_t fsmStateMutex = NULL;

// Initialize in setup()
void setup()
{
  // ...
  fsmStateMutex = xSemaphoreCreateMutex();
  if (fsmStateMutex == NULL)
  {
    Serial.println("ERROR: Failed to create FSM state mutex");
  }
  // ...
}

// Protected access to _stato
void setFsmState(tStati newState)
{
  if (fsmStateMutex != NULL)
  {
    xSemaphoreTake(fsmStateMutex, portMAX_DELAY);
    _stato = newState;
    xSemaphoreGive(fsmStateMutex);
  }
}

tStati getFsmState()
{
  tStati state;
  if (fsmStateMutex != NULL)
  {
    xSemaphoreTake(fsmStateMutex, portMAX_DELAY);
    state = _stato;
    xSemaphoreGive(fsmStateMutex);
  }
  return state;
}
```

---

## 📋 PRIORITY ORDER TO APPLY FIXES

1. **FIX #1** - ISR xHigherPriorityTaskWoken (2 minutes)
2. **FIX #2** - Queue Size Mismatch (1 minute)
3. **FIX #3** - FFT malloc NULL checks (10 minutes)
4. **FIX #4** - MQTT publish timeout (5 minutes)
5. **FIX #5** - parseMessage malloc check (2 minutes)
6. **FIX #7** - detachInterrupt logic (10 minutes)
7. **FIX #6** - SPI transaction endTransaction (5 minutes)
8. **FIX #8** - MQTT timer NULL check (2 minutes)
9. **FIX #9** - Add synchronization (20 minutes) - *Can do later*

**Estimated time to apply critical fixes: ~30 minutes**

---

## 🧪 TESTING AFTER FIXES

```cpp
// Add to main.cpp or setup for testing
void runTests()
{
  Serial.println("=== STARTING TESTS ===");
  
  // Test 1: FFT allocation
  fft_config_t *test_fft = fft_init(4096, FFT_REAL, FFT_FORWARD, NULL, NULL);
  if (test_fft == NULL) {
    Serial.println("ERROR: FFT allocation failed");
  } else {
    Serial.println("✓ FFT allocation successful");
    fft_destroy(test_fft);
  }
  
  // Test 2: Heap free
  Serial.printf("Free heap after FFT test: %d bytes\n", ESP.getFreeHeap());
  
  // Test 3: Queue creation
  if (xQueueADS1256Sample != NULL) {
    Serial.println("✓ ADS1256 queue created");
  } else {
    Serial.println("ERROR: ADS1256 queue not created");
  }
  
  Serial.println("=== TESTS COMPLETE ===");
}
```

---

**Fine Fix Implementation Guide**
