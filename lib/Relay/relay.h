/*
 * Relay.h
 * 
 * Implement a relay controlled by duty cycle.
 * 
 * Author: Rob Bultman
 *         Sept. 13, 2017
 */

#ifndef RELAY_H
#define RELAY_H

#include <stdint.h>

typedef enum {
  kRelayModeManual,
  kRelayModeAutomatic
} RelayMode;

typedef enum {
  kRelayPositionOpen,
  kRelayPositionClosed
} RelayPosition;

class Relay {
  public:
    Relay(uint8_t pin, uint16_t period_in_seconds_);
    void SetRelayMode(RelayMode mode);
    RelayMode GetRelayMode(void);
    void SetRelayPosition(RelayPosition position);
    RelayPosition GetRelayPosition(void);
    void SetDutyCyclePercent(float dutyCycle);
    float GetDutyCyclePercent(void);
    void SetPeriodInSeconds(uint16_t period_in_seconds_);
    uint16_t GetPeriodInSeconds(void);
    void Loop(void);

  private:
    uint8_t pin_;
    uint16_t period_in_seconds_;
    float duty_cycle_;
    RelayMode mode_;
    RelayPosition position_;
    uint32_t period_time_;
    uint32_t old_time_;
};

#endif // RELAY_H