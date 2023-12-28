#include <Arduino.h>
#include <APPLICATION/FFT/FFT_data.h>

//  see: https://www.recordingblogs.com/wiki/welch-window
void welch(float w[], uint32_t len)
{
    uint32_t i;
    float a = (FFT_SIZE - 1) / 2;
    float b = 4.0 / (FFT_SIZE + 1) / (FFT_SIZE + 1);

    for (i = 0; i < FFT_SIZE; i++)
    {
        float c = i - a;
        w[i] = 1.0 - c * c * b;
    }
}