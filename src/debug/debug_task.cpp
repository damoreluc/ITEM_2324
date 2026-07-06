#include <debug/debug_task.h>

#include <esp_heap_caps.h>

void debugTask(void *pvParameters)
{
  (void)pvParameters;

  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t periodTicks = pdMS_TO_TICKS(5000);

  while (1)
  {
    vTaskDelayUntil(&lastWakeTime, periodTicks);

    const uint32_t nowMs = millis();
    const uint32_t uptimeSec = nowMs / 1000UL;
    const uint32_t hours = uptimeSec / 3600UL;
    const uint32_t minutes = (uptimeSec % 3600UL) / 60UL;
    const uint32_t seconds = uptimeSec % 60UL;

    // Network stack is pinned to core 0; Idle0 watermark is a useful core-0 stack health indicator.
    const UBaseType_t netTaskStackHwmWords = uxTaskGetStackHighWaterMark(xTaskGetIdleTaskHandleForCPU(0));
    const uint32_t netTaskStackHwmBytes = (uint32_t)netTaskStackHwmWords * sizeof(StackType_t);

    Serial.printf("[DIAG] Uptime: %02u:%02u:%02u | Free heap: %u B | Min free heap: %u B | Free RAM (internal): %u B | Net task stack HWM: %u words (%u B)\n",
                  hours,
                  minutes,
                  seconds,
                  ESP.getFreeHeap(),
                  ESP.getMinFreeHeap(),
                  heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                  (unsigned)netTaskStackHwmWords,
                  netTaskStackHwmBytes);
  }
}
