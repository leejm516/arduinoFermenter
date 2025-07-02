#include "simplephcontroller.h"

typedef uint8_t (SimplePhController::*EventDetector)();

void SimplePhController::SetOutput() {
    pinMode(base_pin_, OUTPUT);
    pinMode(acid_pin_, OUTPUT);
}

void SimplePhController::DetectEvent() {
    if (current_state_ == kStateIdle) {
        return;
    }

    // ph는 임시로 테스트용 -> 추후 센서로 교체
    current_ph_ = 7.0f + 0.1*sin(2*PI*millis()/60/1000);

    EventDetector event_detectors[] = {
        &SimplePhController::DetectTimeoutAcidOn,
        &SimplePhController::DetectTimeoutAcidOff,
        &SimplePhController::DetectTimeoutBaseOn,
        &SimplePhController::DetectTimeoutBaseOff,
        &SimplePhController::DetectPhHigh,
        &SimplePhController::DetectPhLow
    };   

    const size_t kNumDetectors = sizeof(event_detectors) / sizeof(event_detectors[0]);
    for (uint8_t i = 0; i < kNumDetectors; i++) {
        uint8_t evt = (this->*event_detectors[i])();
        if (evt != kEventNone) {
            last_event_ = evt;
            return;
        }
    }
    last_event_ = kEventNone;
}

void SimplePhController::ProcessState(unsigned long ct) {    
    if (last_event_ == kEventControlOff) {    
        current_state_ = kStateIdle;
        digitalWrite(base_pin_, LOW);
        digitalWrite(acid_pin_, LOW);    
        return;
    }

    switch (current_state_) {
        case kStateIdle:
            if (last_event_ == kEventControlOn) {
                current_state_ = kStateRunning;                
            }
            break;
        case kStateRunning:
            switch (last_event_) {        
                case kEventPhHigh:
                    current_state_ = kStateAcidOn;
                    Serial.println("산 투입을 시작합니다.");
                    digitalWrite(acid_pin_, HIGH);
                    last_on_off_time_ = ct;
                    break;
                case kEventPhLow:
                    current_state_ = kStateAcidOff;
                    Serial.println("염기 투입을 시작합니다.");
                    digitalWrite(base_pin_, HIGH);
                    last_on_off_time_ = ct;
                    break;
                default:
                    break;
            }
            break;
        case kStateBaseOn:
            switch (last_event_) {        
                case kEventTimeout:
                    current_state_ = kStateBaseOff;
                    digitalWrite(base_pin_, LOW);
                    last_on_off_time_ = ct;
                    break;
                default:
                break;
            }      
            break;
        case kStateBaseOff:
            switch (last_event_) {        
                case kEventTimeout:
                    current_state_ = kStateRunning;
                    last_on_off_time_ = ct;
                    break;
                default:
                break;
            }      
            break;
        case kStateAcidOn:
            switch (last_event_) {        
                case kEventTimeout:
                    current_state_ = kStateAcidOff;
                    digitalWrite(acid_pin_, LOW);
                    last_on_off_time_ = ct;
                    break;
                default:
                    break;
            }        
            break;
        case kStateAcidOff:
            switch (last_event_) {        
                case kEventTimeout:
                    current_state_ = kStateRunning;
                    last_on_off_time_ = ct;
                    break;
                default:
                break;
            }      
            break;
        default:
            Serial.println("장비를 점검하세요!");
            break;
        }
}


// timeout 함수는 나중에 하나로 통합 가능할듯
uint8_t SimplePhController::DetectTimeoutAcidOn() {
    return (current_state_ == kStateAcidOff && millis() - last_on_off_time_ >= on_time_) ? kEventTimeout : kEventNone;
}

uint8_t SimplePhController::DetectTimeoutAcidOff() {
    return (current_state_ == kStateAcidOff && millis() - last_on_off_time_ >= on_time_) ? kEventTimeout : kEventNone;
}

uint8_t SimplePhController::DetectTimeoutBaseOn() {
    return (current_state_ == kStateBaseOn && millis() - last_on_off_time_ >= on_time_) ? kEventTimeout : kEventNone;
}

uint8_t SimplePhController::DetectTimeoutBaseOff() {
    return (current_state_ == kStateBaseOff && millis() - last_on_off_time_ >= on_time_) ? kEventTimeout : kEventNone;
}


uint8_t SimplePhController::DetectPhHigh() {
    return (current_ph_ > ph_upper_) ? kEventPhHigh : kEventNone;
}

uint8_t SimplePhController::DetectPhLow() {
    return (current_ph_ < ph_lower_) ? kEventPhLow : kEventNone;
}

void SimplePhController::SetOnTime(float t) {
    on_time_ = (uint8_t)(t*1000);
}
void SimplePhController::SetOffTime(float t) {
    off_time_ = (uint8_t)(t*1000);
}

void SimplePhController::SetControlOn() {
    current_state_ = kStateRunning;
}