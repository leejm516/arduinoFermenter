#include "simplephcontroller.h"

typedef uint8_t (SimplePhController::*EventDetector)();

void SimplePhController::setOutput() {
    pinMode(basepin_, OUTPUT);
    pinMode(acidpin_, OUTPUT);
}

void SimplePhController::detectEvent() {
    if (currentState_ == STATE_IDLE) {
        return;
    }

    // ph는 임시로 테스트용 -> 추후 센서로 교체
    currentPh_ = 7.0f + 0.1*sin(2*PI*millis()/60/1000);

    EventDetector eventDetectors[] = {
        &SimplePhController::detectTimeoutAcidOn,
        &SimplePhController::detectTimeoutAcidOff,
        &SimplePhController::detectTimeoutBaseOn,
        &SimplePhController::detectTimeoutBaseOff,
        &SimplePhController::detectPhHigh,
        &SimplePhController::detectPhLow
    };   

    const size_t numDetectors = sizeof(eventDetectors) / sizeof(eventDetectors[0]);
    for (uint8_t i = 0; i < numDetectors; i++) {
        uint8_t evt = (this->*eventDetectors[i])();
        if (evt != EVENT_NONE) {
            lastEvent_ = evt;
            return;
        }
    }
    lastEvent_ = EVENT_NONE;
}

void SimplePhController::processState(unsigned long ct) {    
    if (lastEvent_ == EVENT_CONTROL_OFF) {    
        currentState_ = STATE_IDLE;
        digitalWrite(basepin_, LOW);
        digitalWrite(acidpin_, LOW);    
        return;
    }

    switch (currentState_) {
        case STATE_IDLE:
            if (lastEvent_ == EVENT_CONTROL_ON) {
                currentState_ = STATE_RUNNING;                
            }
            break;
        case STATE_RUNNING:
            switch (lastEvent_) {        
                case EVENT_PH_HIGH:
                    currentState_ = STATE_ACID_ON;
                    Serial.println("산 투입을 시작합니다.");
                    digitalWrite(acidpin_, HIGH);
                    lastOnOffTime_ = ct;
                    break;
                case EVENT_PH_LOW:
                    currentState_ = STATE_BASE_ON;
                    Serial.println("염기 투입을 시작합니다.");
                    digitalWrite(basepin_, HIGH);
                    lastOnOffTime_ = ct;
                    break;
                default:
                    break;
            }
            break;
        case STATE_BASE_ON:
            switch (lastEvent_) {        
                case EVENT_TIMEOUT:
                    currentState_ = STATE_BASE_OFF;
                    digitalWrite(basepin_, LOW);
                    lastOnOffTime_ = ct;
                    break;
                default:
                break;
            }      
            break;
        case STATE_BASE_OFF:
            switch (lastEvent_) {        
                case EVENT_TIMEOUT:
                    currentState_ = STATE_RUNNING;
                    lastOnOffTime_ = ct;
                    break;
                default:
                break;
            }      
            break;
        case STATE_ACID_ON:
            switch (lastEvent_) {        
                case EVENT_TIMEOUT:
                    currentState_ = STATE_ACID_OFF;
                    digitalWrite(acidpin_, LOW);
                    lastOnOffTime_ = ct;
                    break;
                default:
                    break;
            }        
            break;
        case STATE_ACID_OFF:
            switch (lastEvent_) {        
                case EVENT_TIMEOUT:
                    currentState_ = STATE_RUNNING;
                    lastOnOffTime_ = ct;
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
uint8_t SimplePhController::detectTimeoutAcidOn() {
    return (currentState_ == STATE_ACID_ON && millis() - lastOnOffTime_ >= onTime_) ? EVENT_TIMEOUT : EVENT_NONE;
}

uint8_t SimplePhController::detectTimeoutAcidOff() {
    return (currentState_ == STATE_ACID_OFF && millis() - lastOnOffTime_ >= onTime_) ? EVENT_TIMEOUT : EVENT_NONE;
}

uint8_t SimplePhController::detectTimeoutBaseOn() {
    return (currentState_ == STATE_BASE_ON && millis() - lastOnOffTime_ >= onTime_) ? EVENT_TIMEOUT : EVENT_NONE;
}

uint8_t SimplePhController::detectTimeoutBaseOff() {
    return (currentState_ == STATE_BASE_OFF && millis() - lastOnOffTime_ >= onTime_) ? EVENT_TIMEOUT : EVENT_NONE;
}


uint8_t SimplePhController::detectPhHigh() {
    return (currentPh_ > phUpper_) ? EVENT_PH_HIGH : EVENT_NONE;
}

uint8_t SimplePhController::detectPhLow() {
    return (currentPh_ < phLower_) ? EVENT_PH_LOW : EVENT_NONE;
}

void SimplePhController::setOnTime(float t) {
    onTime_ = (uint8_t)(t*1000);
}
void SimplePhController::setOffTime(float t) {
    offTime_ = (uint8_t)(t*1000);
}

void SimplePhController::setControlOn() {
    currentState_ = STATE_RUNNING;
}