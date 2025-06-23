#ifndef _FERMENTERDATA_H
#define _FERMENTERDATA_H

#include <Arduino.h>

struct PvProfile {
    float temp;
    float ph;    
};

struct SvProfile {
    float temp;
    float phUpper;
    float phLower;
    uint32_t baseOn;
    uint32_t baseOff;
    uint32_t acidOn;
    uint32_t acidOff;
};

void sendPvProfile(PvProfile *p);

#endif