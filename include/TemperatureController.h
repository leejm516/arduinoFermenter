#ifndef _TEMPERATURECONTROLLER_H
#define _TEMPERATURECONTROLLER_H
#include <Arduino.h>
#include "LightweightMAX31865.h"

class TemperatureController {
    public:
        // TemperatureController(uint8_t sp) : tempSensor(MAX31865_RTD::RTD_PT100, sp) {
        //     currentTemp_ = 0.0;
        //     setTemp_ = 37.0;
        // }

        TemperatureController(uint8_t cs, uint8_t mosi, uint8_t miso, uint8_t clk) : tempSensor(cs, mosi, miso, clk) {
            currentTemp_ = 0.0;
            setTemp_ = 37.0;
        }
        
        void init(max31865_numwires_t);

        void update(void);

        float getCurrentTemp(void);

        float getSetTemp(void);
    private:
        LightweightMAX31865 tempSensor;
        float currentTemp_;
        float setTemp_;
       
};


#endif