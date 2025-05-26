#ifndef _SIMPLEPHCONTROLLER_H
#define _SIMPLEPHCONTROLLER_H
#include <Arduino.h>

class SimplePhController {
    public:
        SimplePhController(uint8_t pin1, uint8_t pin2, unsigned long a = 5000, unsigned long b = 5000) {
            basepin_ = pin1;
            acidpin_ = pin2;
            onTime_ = a;
            offTime_ = b;
            currentState_ = STATE_IDLE;
            lastEvent_ = EVENT_NONE;
            currentPh_ = 0.0f;
            phLower_ = 6.95f;
            phUpper_ = 7.05f;
        }

        enum State {
            STATE_IDLE,
            STATE_RUNNING,
            STATE_BASE_ON,
            STATE_BASE_OFF,
            STATE_ACID_ON,
            STATE_ACID_OFF
        };

        enum Event {
            EVENT_NONE,         // 아무 일도 일어나지 않았을 때
            EVENT_CONTROL_ON,
            EVENT_CONTROL_OFF,
            EVENT_PH_HIGH,
            EVENT_PH_LOW,
            EVENT_TIMEOUT
        };


        void setOutput(void);       
        
        void setControlOn(void);

        void detectEvent(void);
        
        // 이벤트 관련 메소드
        uint8_t detectTimeoutAcidOn(void);
        uint8_t detectTimeoutAcidOff(void);
        uint8_t detectTimeoutBaseOn(void);
        uint8_t detectTimeoutBaseOff(void);
        uint8_t detectPhHigh(void);
        uint8_t detectPhLow(void);
        

        // On/Off time 설정
        void setOnTime(float);
        void setOffTime(float);

        void processState(unsigned long);
        
        // 멤버 변수값 출력 함수
        float getCurrentPh() { return currentPh_; };
        uint8_t getLastEvent() { return lastEvent_; };
        uint8_t getCurrentState() { return currentState_; };

    private:
        uint8_t currentState_;
        uint8_t lastEvent_;
        unsigned long lastOnOffTime_;
        uint8_t basepin_;
        uint8_t acidpin_;
        unsigned long onTime_;
        unsigned long offTime_;
        float phUpper_;
        float phLower_;
        float currentPh_;
};


#endif