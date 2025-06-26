/*
 * Relay.h
 * 
 * Implement a relay controlled by duty cycle.
 * 
 * Author: Rob Bultman
 *         Sept. 13, 2017
 */

#include "relay.h"
#include <Arduino.h>

Relay::Relay(uint8_t pin, uint16_t period_in_seconds): pin_(pin), period_in_seconds_(period_in_seconds) {
    pinMode(pin_, OUTPUT);
    digitalWrite(pin_, LOW);
    duty_cycle_ = 0.5;
    mode_ = kRelayModeManual;
    position_ = kRelayPositionOpen;
}

void Relay::setRelayMode(RelayMode mode) {
   switch (mode) {
      case kRelayModeManual:
         mode_ = mode;
         setRelayPosition(kRelayPositionOpen);
         break;
      case kRelayModeAutomatic:
         if (mode_ != kRelayModeAutomatic) {
            mode_ = mode;
            period_time_ = 0;
            old_time_ = millis();
         }
         break;
   }
}

RelayMode Relay::getRelayMode(void) {
   return mode_;
}

void Relay::setRelayPosition(RelayPosition position) {
   switch(position) {
      case kRelayPositionOpen:
      case kRelayPositionClosed:
         position_ = position;
         digitalWrite(pin_, position_);
         break;
   }
}

RelayPosition Relay::getRelayPosition(void) {
  return position_;
}

void Relay::setDutyCyclePercent(double dutyCycle) {
   if ((dutyCycle >= 0.0) && (dutyCycle <= 1.0)) {
      duty_cycle_ = dutyCycle;
   }
}

double Relay::getDutyCyclePercent(void) {
   return duty_cycle_;  
}

void Relay::setPeriodInSeconds(uint16_t period_in_seconds) {
   period_in_seconds_ = period_in_seconds;
}

uint16_t Relay::getPeriodInSeconds(void) {
   return period_in_seconds_;
}

void Relay::loop(void) {
   uint32_t newTime = millis();
   uint32_t offTime = period_in_seconds_ * 1000 * duty_cycle_;

   if (mode_ == kRelayModeManual) {
      return;
   }

   if (newTime < old_time_) {
      period_time_ += (UINT32_MAX - old_time_ + newTime);
   } else {
      period_time_ += (newTime - old_time_);
   }
   old_time_ = newTime;

   if (period_time_ < offTime) {
      if (duty_cycle_ > 0.0) setRelayPosition(kRelayPositionClosed);
   } else if (period_time_ >= period_in_seconds_*1000) {
      period_time_ = 0;
      if (duty_cycle_ > 0.0) setRelayPosition(kRelayPositionClosed);
   } else {
      setRelayPosition(kRelayPositionOpen);
   }
}
