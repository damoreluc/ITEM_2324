#include <WIFI/wifi_functions.h>

#include <Preferences.h>
#include <WiFiManager.h>

namespace
{
const char *kPrefsNamespace = "item_cfg";
const char *kKeyBroker = "mqtt_broker";
const char *kKeyPort = "mqtt_port";
const char *kKeyUser = "mqtt_user";
const char *kKeyPass = "mqtt_pass";
const char *kKeyClient = "mqtt_client";

bool g_saveConfigRequested = false;

void onWmSaveConfig()
{
  g_saveConfigRequested = true;
}

void copyString(char *dst, size_t dstSize, const String &src)
{
  if (dstSize == 0)
  {
    return;
  }

  strlcpy(dst, src.c_str(), dstSize);
}

void copyString(char *dst, size_t dstSize, const char *src)
{
  if (dstSize == 0)
  {
    return;
  }

  strlcpy(dst, src == NULL ? "" : src, dstSize);
}

uint16_t parsePort(const char *portText, uint16_t fallback)
{
  if (portText == NULL || portText[0] == '\0')
  {
    return fallback;
  }

  long parsed = strtol(portText, NULL, 10);
  if (parsed <= 0 || parsed > 65535)
  {
    return fallback;
  }

  return (uint16_t)parsed;
}
} // namespace

bool resetSavedNetworkParameters()
{
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, false))
  {
    Serial.println(F("[WiFiManager] Failed to open preferences namespace"));
    return false;
  }

  prefs.clear();
  prefs.end();

  WiFiManager wm;
  wm.resetSettings();

  // Clear STA config from NVS so next boot forces a full reprovisioning.
  WiFi.disconnect(true, true);

  Serial.println(F("[WiFiManager] Saved network parameters reset"));
  return true;
}

bool runWiFiManagerBlocking(stMqttRuntimeConfig &cfg,
                            const char *defaultBroker,
                            uint16_t defaultPort,
                            const char *defaultUser,
                            const char *defaultPassword,
                            const char *defaultClientId)
{
  Preferences prefs;
  prefs.begin(kPrefsNamespace, false);

  const String savedBroker = prefs.getString(kKeyBroker, defaultBroker == NULL ? "" : defaultBroker);
  const uint16_t savedPort = prefs.getUShort(kKeyPort, defaultPort);
  const String savedUser = prefs.getString(kKeyUser, defaultUser == NULL ? "" : defaultUser);
  const String savedPassword = prefs.getString(kKeyPass, defaultPassword == NULL ? "" : defaultPassword);
  const String savedClientId = prefs.getString(kKeyClient, defaultClientId == NULL ? "" : defaultClientId);

  char brokerBuf[64];
  char portBuf[8];
  char userBuf[64];
  char passwordBuf[64];
  char clientIdBuf[32];

  copyString(brokerBuf, sizeof(brokerBuf), savedBroker);
  snprintf(portBuf, sizeof(portBuf), "%u", (unsigned)savedPort);
  copyString(userBuf, sizeof(userBuf), savedUser);
  copyString(passwordBuf, sizeof(passwordBuf), savedPassword);
  copyString(clientIdBuf, sizeof(clientIdBuf), savedClientId);

  WiFiManager wm;
  wm.setConfigPortalBlocking(true);
  wm.setSaveConfigCallback(onWmSaveConfig);

  WiFiManagerParameter brokerParam("mqtt_broker", "MQTT broker", brokerBuf, sizeof(brokerBuf));
  WiFiManagerParameter portParam("mqtt_port", "MQTT port", portBuf, sizeof(portBuf));
  WiFiManagerParameter userParam("mqtt_user", "MQTT user", userBuf, sizeof(userBuf));
  WiFiManagerParameter passParam("mqtt_pass", "MQTT password", passwordBuf, sizeof(passwordBuf), "type='password'");
  WiFiManagerParameter clientParam("mqtt_client", "MQTT client id", clientIdBuf, sizeof(clientIdBuf));

  wm.addParameter(&brokerParam);
  wm.addParameter(&portParam);
  wm.addParameter(&userParam);
  wm.addParameter(&passParam);
  wm.addParameter(&clientParam);

  g_saveConfigRequested = false;

  Serial.println(F("[WiFiManager] Starting blocking provisioning"));
  const bool connected = wm.autoConnect("ITEM_ConfigAP");

  if (!connected)
  {
    Serial.println(F("[WiFiManager] Provisioning failed or timed out"));
    wm.stopWebPortal();
    WiFi.softAPdisconnect(true);
    prefs.end();
    return false;
  }

  copyString(cfg.broker, sizeof(cfg.broker), brokerParam.getValue());
  copyString(cfg.user, sizeof(cfg.user), userParam.getValue());
  copyString(cfg.password, sizeof(cfg.password), passParam.getValue());
  copyString(cfg.clientId, sizeof(cfg.clientId), clientParam.getValue());
  cfg.port = parsePort(portParam.getValue(), defaultPort);

  if (cfg.broker[0] == '\0')
  {
    copyString(cfg.broker, sizeof(cfg.broker), defaultBroker);
  }
  if (cfg.clientId[0] == '\0')
  {
    copyString(cfg.clientId, sizeof(cfg.clientId), defaultClientId);
  }

  if (g_saveConfigRequested)
  {
    prefs.putString(kKeyBroker, cfg.broker);
    prefs.putUShort(kKeyPort, cfg.port);
    prefs.putString(kKeyUser, cfg.user);
    prefs.putString(kKeyPass, cfg.password);
    prefs.putString(kKeyClient, cfg.clientId);
    Serial.println(F("[WiFiManager] MQTT parameters saved"));
  }

  wm.stopWebPortal();
  WiFi.softAPdisconnect(true);
  prefs.end();

  Serial.println(F("[WiFiManager] Provisioning completed"));
  return true;
}
