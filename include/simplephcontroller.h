#ifndef _SIMPLEPHCONTROLLER_H
#define _SIMPLEPHCONTROLLER_H
#include <Arduino.h>

class SimplePhController {
    public:
        SimplePhController(uint8_t pin_1, uint8_t pin_2, unsigned long a = 5000, unsigned long b = 5000) {
            base_pin_ = pin_1;
            acid_pin_ = pin_2;
            on_time_ = a;
            off_time_ = b;
            current_state_ = kStateIdle;
            last_event_ = kEventNone;
            current_ph_ = 0.0f;
            ph_lower_ = 6.95f;
            ph_upper_ = 7.05f;
        }

        enum State {
            kStateIdle,
            kStateRunning,
            kStateBaseOn,
            kStateBaseOff,
            kStateAcidOn,
            kStateAcidOff
        };

        enum Event {
            kEventNone,         // 아무 일도 일어나지 않았을 때
            kEventControlOn,
            kEventControlOff,
            kEventPhHigh,
            kEventPhLow,
            kEventTimeout
        };


        void SetOutput(void);       
        
        void SetControlOn(void);

        void DetectEvent(void);
        
        // 이벤트 관련 메소드
        uint8_t DetectTimeoutAcidOn(void);
        uint8_t DetectTimeoutAcidOff(void);
        uint8_t DetectTimeoutBaseOn(void);
        uint8_t DetectTimeoutBaseOff(void);
        uint8_t DetectPhHigh(void);
        uint8_t DetectPhLow(void);
        

        // On/Off time 설정
        void SetOnTime(float);
        void SetOffTime(float);

        void ProcessState(unsigned long);
        
        // 멤버 변수값 출력 함수
        float GetCurrentPh() { return current_ph_; };
        uint8_t GetLastEvent() { return last_event_; };
        uint8_t GetCurrentState() { return current_state_; };

    private:
        uint8_t current_state_;
        uint8_t last_event_;
        unsigned long last_on_off_time_;
        uint8_t base_pin_;
        uint8_t acid_pin_;
        unsigned long on_time_;
        unsigned long off_time_;
        float ph_upper_;
        float ph_lower_;
        float current_ph_;
};


#endif