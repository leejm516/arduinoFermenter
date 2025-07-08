#ifndef _TEMPERATURECONTROLLER_H
#define _TEMPERATURECONTROLLER_H
#include <Arduino.h>
#include "lightweightmax31865.h"

class TemperatureController {
    public:
        // TemperatureController(uint8_t sp) : tempSensor(MAX31865_RTD::RTD_PT100, sp) {
        //     currentTemp_ = 0.0;
        //     setTemp_ = 37.0;
        // }

        TemperatureController(uint8_t cs) : tempSensor(cs) {
            current_temp_ = 0.0;
            set_temp_ = 37.0;
        }
        
        void init(max31865_numwires_t);

        void init(max31865_numwires_t, float);

        void update(void);

        float getCurrentTemp(void);

        float getSetTemp(void);

        void setTemp(float);
    private:
        LightweightMAX31865 tempSensor;
        float current_temp_;
        float set_temp_;
       
};


#endif