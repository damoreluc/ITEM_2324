#include <APPLICATION\ADS1256\ADS1256.h>
#include <APPLICATION\ADS1256\ADS1256Ext.h>
#include <APPLICATION\HWCONFIG\hwConfig.h>

#define ADS1256_ADCON 0x02

// constructor
ADS1256Ext::ADS1256Ext() : ADS1256() {} 

// initial setup
void ADS1256Ext::setup() {
    // configurazione iniziale ADC accelerometri ADS1256
    // imposta la isr dedicata al data ready dell'ADC ADS1256, triggerata sul fronte di discesa dell'interrupt
    // configurazione dell'ADS1256 e delle sue linee di controllo
    //  NB: SPI clock <= F_clkin / 4 = 7.68e6 / 4 = 1920000
    adc.init(hspi, nCS, nDRDY, nPDWN, 1900000);
    // mux between accelerometer channels now is done by PGA
    adc.setChannel(adc.ads1256_mux[0]);
    //adc.setChannel(channels[0]);
    // set internal pga gain x1
    adc.setGain(ADS1256Ext::PGA_2);
    // put adc in stand-by
    adc.standby();
}

// set PGA gain
void  ADS1256Ext::setGain(pgaGains gain) {
    // read ADCON register
    byte adcon_reg = ADS1256_ADCON;
    byte adcon_data = readRegister(adcon_reg);

    // update gain bits
    adcon_data = (adcon_data & 0xF8) | gain;

    // write back ADC register
    writeRegister(adcon_reg, adcon_data);

}

// ADC instance
ADS1256Ext adc;

// ADS1256 DRDY
volatile bool newData = false;
volatile uint16_t countData = 0;