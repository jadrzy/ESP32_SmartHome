#include "light_sensor.h"

//****************************************************************************
// TAOS, Inc. 2004−2005
// THIS CODE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//****************************************************************************

// Scaling constants for lux calculation
#define LUX_SCALE 14        // Lux scale factor: scale by 2^14
#define RATIO_SCALE 9       // Ratio scale factor: scale ratio by 2^9

//****************************************************************************
// Integration time scaling factors
//****************************************************************************
#define CH_SCALE 10         // Scale channel values by 2^10
#define CHSCALE_TINT0 0x7517 // Scale for 13.7ms integration time (322/11 * 2^CH_SCALE)
#define CHSCALE_TINT1 0x0fe7 // Scale for 101ms integration time (322/81 * 2^CH_SCALE)

//****************************************************************************
// Coefficients for lux calculation (T and FN package coefficients)
//****************************************************************************
// Piecewise approximation for Ch1/Ch0 ratio

#define K1T 0x0040 // 0.125 * 2^RATIO_SCALE
#define B1T 0x01f2 // 0.0304 * 2^LUX_SCALE
#define M1T 0x01be // 0.0272 * 2^LUX_SCALE

#define K2T 0x0080 // 0.250 * 2^RATIO_SCALE
#define B2T 0x0214 // 0.0325 * 2^LUX_SCALE
#define M2T 0x02d1 // 0.0440 * 2^LUX_SCALE

#define K3T 0x00c0 // 0.375 * 2^RATIO_SCALE
#define B3T 0x023f // 0.0351 * 2^LUX_SCALE
#define M3T 0x037b // 0.0544 * 2^LUX_SCALE

#define K4T 0x0100 // 0.500 * 2^RATIO_SCALE
#define B4T 0x0270 // 0.0381 * 2^LUX_SCALE
#define M4T 0x03fe // 0.0624 * 2^LUX_SCALE

#define K5T 0x0138 // 0.610 * 2^RATIO_SCALE
#define B5T 0x016f // 0.0224 * 2^LUX_SCALE
#define M5T 0x01fc // 0.0310 * 2^LUX_SCALE

#define K6T 0x019a // 0.800 * 2^RATIO_SCALE
#define B6T 0x00d2 // 0.0128 * 2^LUX_SCALE
#define M6T 0x00fb // 0.0153 * 2^LUX_SCALE

#define K7T 0x029a // 1.30 * 2^RATIO_SCALE
#define B7T 0x0018 // 0.00146 * 2^LUX_SCALE
#define M7T 0x0012 // 0.00112 * 2^LUX_SCALE

#define K8T 0x029a // Ratio > 1.3
#define B8T 0x0000 // No Lux for this ratio range
#define M8T 0x0000 // No Lux for this ratio range

//****************************************************************************
// Function: unsigned int CalculateLux(unsigned int ch0, unsigned int ch1, int iType)
// Description: Calculate approximate illuminance (lux) based on raw sensor values
// using a piecewise linear approximation of the equation.
// Arguments:
//    iGain - Gain setting (0: 1X, 1: 16X)
//    tInt - Integration time (0: 13.7ms, 1: 101ms, 2: 402ms, 3: Manual)
//    ch0 - Raw channel 0 value
//    ch1 - Raw channel 1 value
//    iType - Sensor package type (T or CS)
// Return: Approximate illuminance in lux
//****************************************************************************
static unsigned int CalculateLux(unsigned int iGain, unsigned int tInt, unsigned int ch0, unsigned int ch1)
{
    unsigned long chScale;
    unsigned long channel0;
    unsigned long channel1;

    // Scale channel values based on the integration time
    switch (tInt)
    {
        case 0:  // 13.7ms integration time
            chScale = CHSCALE_TINT0;
            break;
        case 1:  // 101ms integration time
            chScale = CHSCALE_TINT1;
            break;
        default: // No scaling for 402ms
            chScale = (1 << CH_SCALE);
            break;
    }

    // Scale the values if gain is not 16X
    if (!iGain) chScale = chScale << 4; // Scale 1X to 16X

    // Scale channel 0 and channel 1 values
    channel0 = (ch0 * chScale) >> CH_SCALE;
    channel1 = (ch1 * chScale) >> CH_SCALE;

    // Calculate ratio of channel values (channel1/channel0) and protect against divide-by-zero
    unsigned long ratio1 = 0;
    if (channel0 != 0) ratio1 = (channel1 << (RATIO_SCALE + 1)) / channel0;

    // Round the ratio
    unsigned long ratio = (ratio1 + 1) >> 1;

    // Choose appropriate coefficients based on the ratio
    unsigned int b = 0, 
                 m = 0;
    
    if (ratio <= K1T) {
        b = B1T; m = M1T;
    } else if (ratio <= K2T) {
        b = B2T; m = M2T;
    } else if (ratio <= K3T) {
        b = B3T; m = M3T;
    } else if (ratio <= K4T) {
        b = B4T; m = M4T;
    } else if (ratio <= K5T) {
        b = B5T; m = M5T;
    } else if (ratio <= K6T) {
        b = B6T; m = M6T;
    } else if (ratio <= K7T) {
        b = B7T; m = M7T;
    } else {
        b = B8T; m = M8T; // Ratio greater than K8T
    }

    // Calculate lux: lux = (ch0 * b) - (ch1 * m)
    unsigned long temp;
    temp = ((channel0 * b) - (channel1 * m));

    // Add rounding factor (2^(LUX_SCALE−1))
    temp += (1 << (LUX_SCALE - 1));

    // Remove the fractional portion
    unsigned long lux = temp >> LUX_SCALE;

    return lux;
}
