#include "bldcmotor.h"

void BldcMotor::update(unsigned long period) {
    // 일단 홀센서 출력에서 발생시키는 인터럽트를 받아서 float 배열에 최근 5포인트 수치를 가지고 가중평균으로 rpm을 계산하는 식으로 작성해 본다.
    // 엔코더가 아니기 때문에 단일 밸류는 fluctuation이 심함.
    // 연습시에는 따로 ISR에 배열 부분을 구현하였지만 loop 한 cycle 돌때만 period를 계산해도 충분하지 않을까 하는 생각.
    cli();    
    for (uint8_t i = 0; i < 4; i++ ) {
        periods_[i] = periods_[i+1];
    }
    periods_[4] = period;
    sei();
}

void BldcMotor::setRpm(unsigned int rpm) {
    if (rpm >= 0 && rpm <= 1000) {
        set_rpm_ = rpm;
    } else {
        Serial.println("RPM 지정 범위를 초과하였습니다");
    }    
}

unsigned int BldcMotor::getSetRpm() {
    return set_rpm_;
}


void BldcMotor::setOutput(unsigned int output) {    
    if (output >= 0 && output <= 999) {
        output_ = output;
    }

    if (output > 999) {
        output_ = 999;
    }

    if (output < 0) {
        output_ = 0;
    }

    OCR4A = output_;
}

unsigned int BldcMotor::getCurrentRpm() {
    return current_rpm_;
}

unsigned long BldcMotor::getLastTime() {
    return last_time_;
}

void BldcMotor::setLastTime(unsigned long last_time) {
    last_time_ = last_time;
}

unsigned long BldcMotor::calculateMeanPeriod() {    
    unsigned long sum_period = 0;
    cli();
    for (uint8_t i = 0; i < 5; i++) {
        sum_period += periods_[i];
    }
    sei();
    return (unsigned long)(sum_period / 5 + 0.5);
}

void BldcMotor::calculateRpm() {
    float frequency = 1000000.0 / calculateMeanPeriod();
    float rpm = frequency / 3 / 4 / 4 * 60;

    current_rpm_ = (unsigned int)(rpm + 0.5);
}