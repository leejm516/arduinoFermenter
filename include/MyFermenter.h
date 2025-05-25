#ifndef _MYFERMENTER_H
#define _MYFERMENTER_H

#include <Arduino.h>
#include <ControlLoop.h>

class MyFermenter {
    public:
        MyFermenter() {
            basePumpPin_ = 48;
            coolerPin_ = 50;
            heaterPin_ = 51;

            currentPh_ = 0.0f;
            currentDO_ = 0.0f;
            currentTemp_ = 0.0f;
            currentRpm_ = 0;

            nowTime_ = 0;
            lastBaseTime_ = 0;

            baseOnInterval_ = 1000;
            baseOffInterval_ = 9000;
            basePumpOn_ = false;
            basePumpRunning_ = false;            
        }

        void update(void);

        float getCurrentPh() { return currentPh_; }

        void onBaseControl(void);
        
        void offBaseControl(void);

        void setBaseOnInterval(unsigned long); // Time in milliseconds

        void setBaseOffInterval(unsigned long);

        void runBasePump(double);
        
        float getCurrentTemp() { return currentTemp_; }

        uint8_t getCoolerPin() { return coolerPin_; }

        void setCoolerPin(uint8_t p) { coolerPin_ = p; }

        uint8_t getHeaterPin() { return heaterPin_; }

        void setHeaterPin(uint8_t p) { heaterPin_ = p; }
                
    private:
        unsigned long nowTime_ = millis();
        unsigned long lastUpdateCurrentTime_ = millis();
        
        uint8_t basePumpPin_;
        // uint8_t acidPumpPin_;
        // uint8_t motorPin_;
        uint8_t coolerPin_;
        uint8_t heaterPin_;

        float currentPh_;
        
        // in degC
        float currentTemp_;
        
        // in percents
        float currentDO_; 
                
        uint16_t currentRpm_;        

        // 펌프 on/off
        unsigned long lastBaseTime_ ; // 펌프 상태가 바뀐 시간을 기억하는 변수
        unsigned long baseOnInterval_;
        unsigned long baseOffInterval_;        
        bool basePumpOn_;
        bool basePumpRunning_;

        // Heater 관련 변수
        float heaterKp_;
        float heaterKi_;
        float heaterKd_;
        
};


#endif