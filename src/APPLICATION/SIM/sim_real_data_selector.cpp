#include <APPLICATION\SIM\sim_real_data_selector.h>

eSensMode sensMode = SYM_DATA;

// reads SENS_MODE pin and updates simulated_data/real_data state
void readSensMode()
{
    pinMode(SENS_MODE, INPUT_PULLUP);

    if (digitalRead(SENS_MODE) == HIGH)
    {
        sensMode = REAL_DATA;
    } else {
        sensMode = SYM_DATA;
    }
}

// returns simulated_data/real_data state
eSensMode getSensMode()
{
    return sensMode;
}