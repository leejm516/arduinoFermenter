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
    void setRelayMode(RelayMode mode);
    RelayMode getRelayMode(void);
    void setRelayPosition(RelayPosition position);
    RelayPosition getRelayPosition(void);
    void setDutyCyclePercent(double dutyCycle);
    double getDutyCyclePercent(void);
    void setPeriodInSeconds(uint16_t period_in_seconds_);
    uint16_t getPeriodInSeconds(void);
    void loop(void);

  private:
    uint8_t pin_;
    uint16_t period_in_seconds_;
    double duty_cycle_;
    RelayMode mode_;
    RelayPosition position_;
    uint32_t period_time_;
    uint32_t old_time_;
};

#endif // RELAY_H