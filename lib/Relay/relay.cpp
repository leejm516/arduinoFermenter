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

void Relay::SetRelayMode(RelayMode mode) {
   switch (mode) {
      case kRelayModeManual:
         mode_ = mode;
         SetRelayPosition(kRelayPositionOpen);
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

RelayMode Relay::GetRelayMode(void) {
   return mode_;
}

void Relay::SetRelayPosition(RelayPosition position) {
   switch(position) {
      case kRelayPositionOpen:
      case kRelayPositionClosed:
         position_ = position;
         digitalWrite(pin_, position_);
         break;
   }
}

RelayPosition Relay::GetRelayPosition(void) {
  return position_;
}

void Relay::SetDutyCyclePercent(float duty_cycle) {
   if ((duty_cycle >= 0.0) && (duty_cycle <= 1.0)) {
      duty_cycle_ = duty_cycle;
   }
}

float Relay::GetDutyCyclePercent(void) {
   return duty_cycle_;  
}

void Relay::SetPeriodInSeconds(uint16_t period_in_seconds) {
   period_in_seconds_ = period_in_seconds;
}

uint16_t Relay::GetPeriodInSeconds(void) {
   return period_in_seconds_;
}

void Relay::Loop(void) {
	// This function updates the relay status based on the elapsed time with a duty cycle and a period given

	if (mode_ == kRelayModeManual) {
		return;
        }

	const uint32_t now = millis();
	const uint32_t elapsed = now - old_time_;
	old_time_ = now;

	const uint32_t period_ms = 
		static_cast<uint32_t>(period_in_seconds_) * 1000UL;

	// fail-safe
	if (period_ms == 0) {
		period_time_ = 0;
		SetRelayPosition(kRelayPositionOpen);
		return;
	}

	period_time_ = (period_time_ + elapsed % period_ms) % period_ms;

	if (duty_cycle_ <= 0.0f) {
		SetRelayPosition(kRelayPositionOpen);
        	return;
	}

	if (duty_cycle_ >= 1.0f) {
		SetRelayPosition(kRelayPositionClosed);
		return;
	}

	const uint32_t on_time_ms =
		static_cast<uint32_t>(
				static_cast<float>(period_ms) * duty_cycle_);

	SetRelayPosition(
			period_time_ < on_time_ms
			? kRelayPositionClosed
			: kRelayPositionOpen
			);
}
