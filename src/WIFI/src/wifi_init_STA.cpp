#include <WIFI/wifi_functions.h>
#include <APPLICATION\HWCONFIG\hwConfig.h>

void initWiFi_STA()
{
  // comanda un led per indicare la connessione all'access point WiFi
  // pinMode(pinWiFiConnected, OUTPUT);

  // WiFi is configured to run on core 0 (PRO_CPU) via platformio.ini build flags:
  // CONFIG_LWIP_CORE=0, CONFIG_ESP_WIFI_TASK_CORE_ID=0, CONFIG_ESP_WIFI_TASK_AFFINITY=0x1
  WiFi.mode(WIFI_STA);
  
  Serial.println(F("[WiFi] WiFi configured for core 0 operation"));

  // Register only the callbacks needed by this project lifecycle.
  WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println(F("[WiFi] Connecting to WiFi "));

  if (getSensMode() == REAL_DATA)
  {
    ssd1306_publish("Connecting to WiFi\n");
  }

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.printf(" \n");
}