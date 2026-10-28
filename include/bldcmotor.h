#ifndef _BLDCMOTOR_H
#define _BLDCMOTOR_H

#include <Arduino.h>
#include "controlloop.h"

// BLDC agitation motor: PWM output on D6 (Timer4/OC4A, Arduino Mega) and
// speed feedback from the driver's pulse output (one ISR call per rising edge).
// Also usable directly as the DataSource / RelayUpdate of a ControlLoop,
// so no adapter classes are needed in main.cpp.
class BldcMotor : public DataSource, public RelayUpdate {
    public:
        static const uint16_t kMaxRpm = 1000;         // upper limit for the setpoint
        static const uint16_t kMaxOutput = 999;       // PWM duty range 0..999 (Timer4 TOP = ICR4)

        // Configure the pins and Timer4 (2 kHz fast PWM). Call once in setup().
        void Begin(uint8_t signal_pin);

        // Record one feedback pulse. Call from the rising-edge ISR only.
        void OnEdge();

        // Convert the recorded pulse periods into current_rpm_. Call from the main loop.
        void CalculateRpm();

        // RPM setpoint (clamped to kMaxRpm); the PID in ControlLoop uses it as its target.
        void SetRpm(uint16_t rpm) { set_rpm_ = min(rpm, kMaxRpm); }
        uint16_t GetSetRpm() const { return set_rpm_; }

        // Last RPM computed by CalculateRpm().
        uint16_t GetCurrentRpm() const { return current_rpm_; }

        // Write the PWM duty (0..kMaxOutput, clamped) to the motor driver.
        void SetOutput(uint16_t output);

        // DataSource / RelayUpdate interface for ControlLoop
        float Get() { return current_rpm_; }                        // process variable for the PID
        void On() {}                                                // nothing to do; PWM runs continuously
        void Off() { SetOutput(0); }                                // stop the motor
        void Update(float output) { SetOutput((uint16_t)output); }  // PID output -> PWM duty

    private:
        static const uint8_t kPulsesPerRev = 3 * 4 * 4;             // feedback pulses per shaft revolution
        static const uint8_t kSamples = 10;                         // moving-average length
        static const unsigned long kMinPeriodUs = 600;              // edges closer than this are noise (> 1000 RPM)
        static const unsigned long kStallTimeoutUs = 500000;        // no edge for this long -> 0 RPM

        uint16_t current_rpm_ = 0;
        uint16_t set_rpm_ = 0;

        // Shared between the ISR and the main loop, hence volatile.
        volatile unsigned long last_edge_us_ = 0;     // micros() of the last accepted edge
        volatile unsigned long periods_[kSamples] = {}; // ring buffer of pulse periods [us]
        volatile uint8_t head_ = 0;                   // next slot to overwrite
        volatile uint8_t count_ = 0;                  // valid samples (<= kSamples)
};

#endif
