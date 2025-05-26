#ifndef _TEMPERATURECONTROLLER_H
#define _TEMPERATURECONTROLLER_H
#include <Arduino.h>
#include <SPI.h>
#include "MAX31865.h"

class TemperatureController {
    public:
        TemperatureController(uint8_t sp) : tempSensor(MAX31865_RTD::RTD_PT100, sp) {
            currentTemp_ = 0.0;
            setTemp_ = 37.0;
        }
        
        void init(void);

        void update(void);

        float getCurrentTemp(void);

        float getSetTemp(void);
    private:
        MAX31865_RTD tempSensor;
        float currentTemp_;
        float setTemp_;
       
};


#endif