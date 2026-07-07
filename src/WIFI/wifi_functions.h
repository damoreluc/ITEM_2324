#ifndef _WIFI_FUNCTIONS_H
#define _WIFI_FUNCTIONS_H

#include <Arduino.h>
#include <WiFi.h>
#include <APPLICATION/SSD1306/ssd1306.h>
#include <APPLICATION\SIM\sim_real_data_selector.h>

typedef struct stMqttRuntimeConfig
{
	char broker[64];
	uint16_t port;
	char user[64];
	char password[64];
	char clientId[32];
} stMqttRuntimeConfig;

// avvia provisioning bloccante con WiFiManager e carica i parametri MQTT
// when it returns true, the device is connected to WiFi and config is ready
bool runWiFiManagerBlocking(stMqttRuntimeConfig &cfg,
														const char *defaultBroker,
														uint16_t defaultPort,
														const char *defaultUser,
														const char *defaultPassword,
														const char *defaultClientId);

// clears stored WiFi credentials (NVS) and MQTT parameters saved by WiFiManager flow
bool resetSavedNetworkParameters();

// configures WiFi module as STATION
// and connects to an Access Point with credentials
// stored in persistent WiFi stack configuration
void initWiFi_STA();

// WiFi event handling
void WiFiEvent(WiFiEvent_t event);

// Prints the obtained IP address
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);

// Prints confirmation of access-point connection
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);

// Handles disconnection and reconnection attempt
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

// Operations to perform on upper layers when IP layer is ready
void WiFiNetworkReady();

// Operations to perform on upper layers when IP layer is down
void WiFiNetworkFail();

#endif
