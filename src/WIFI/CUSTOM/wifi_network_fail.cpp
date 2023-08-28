#include <WIFI/wifi_functions.h>
#include <MQTT/mqtt_functions.h>

// Operazioni da compiere sui layers superiori se il layer IP è caduto
void WiFiNetworkFail() {
    // ferma il monitoraggio della connessione MQTT mentre 
    // la connessione WiFi è in fase di ripristino
    stopTimersRTOS();
}