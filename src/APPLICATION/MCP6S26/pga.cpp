#include <APPLICATION\MCP6S26\pga.h>
#include <APPLICATION\MCP6S26\MCP6S26.h>

// tabella guadagni del PGA MCP6S26
uint8_t gainValues[] = {1, 2, 4, 5, 8, 10, 16, 32};

// impostazioni del PGA0
stPGA pga0 = {.channel = MCP6S26_CH1, .gain = MCP6S26_GAIN_1, .gainValue=gainValues[MCP6S26_GAIN_1] , .gain_changed = true};

// comando del guadagno del PGA
// è arrivato un messaggio da pgaSetGainTopic
// deve essere un valore tra 0 e 7; forzato a 0 se esterno all'intervallo o valore non numerico
void setPGAgain(char *data)
{
    uint8_t g;
    int k;
    
    //Serial.print("pga data sel: ");
    //Serial.println(data);

    // se il payload MQTT contiene più di una cifra
    // forza il PGA al guadagno minimo
    if (strlen(data) > 1)
    {
        k = 0;
    }
    else
    {
        k = data[0] - '0';
    }
    
    // k deve essere un valore tra 0 e 7; 
    // forzato a 0 se esterno all'intervallo o valore non numerico
    if (k < 0 || k > (sizeof(MCP6S26_gains) - 1))
    {
        g = MCP6S26_gains[0];
    }
    else
    {
        g = MCP6S26_gains[k];
    }

    if (g != pga0.gain)
    {
        pga0.gain = g;
        pga0.gainValue = gainValues[g];
        pga0.gain_changed = true;
    }
    
    Serial.printf("\nSelettore guadagno = %d\tGuadagno = %d\n", pga0.gain, pga0.gainValue);
}