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

        // Begin(signal_pin)
        //   Sets up the hardware: the feedback pin as input (with pull-up), D6 as output,
        //   and Timer4 as a 2 kHz PWM generator with 0% duty (motor stopped).
        //   Call once in setup(). Attaching the ISR (attachInterrupt) is left to the caller,
        //   because it needs a plain function that forwards to OnEdge().
        void Begin(uint8_t signal_pin);

        // OnEdge()
        //   Handles one feedback pulse: measures the time since the previous pulse and
        //   stores it for averaging. Noise edges are ignored. Does no RPM math,
        //   so it stays fast enough for an ISR. Call from the rising-edge ISR only.
        void OnEdge();

        // CalculateRpm()
        //   Averages the stored pulse periods and converts them to RPM, updating the value
        //   returned by GetCurrentRpm() / Get(). Sets 0 RPM if the motor has stopped.
        //   Call repeatedly from the main loop (not from an ISR).
        void CalculateRpm();

        // SetRpm(rpm) / GetSetRpm()
        //   Store / read the target speed. Values above kMaxRpm are clamped.
        //   This only stores the number; main.cpp passes it to the PID via
        //   motor_control_loop.SetPoint(). Nothing is written to the motor here.
        void SetRpm(uint16_t rpm) { set_rpm_ = min(rpm, kMaxRpm); }
        uint16_t GetSetRpm() const { return set_rpm_; }

        // GetCurrentRpm()
        //   Returns the most recent measured speed. It only changes when CalculateRpm() runs.
        uint16_t GetCurrentRpm() const { return current_rpm_; }

        // SetOutput(output)
        //   Sets the PWM duty sent to the motor driver: 0 = stopped, kMaxOutput = full power.
        //   Values above kMaxOutput are clamped. Takes effect immediately.
        void SetOutput(uint16_t output);

        // --- Interface used by ControlLoop (see controlloop.h) ---
        // Get():    the measured value (RPM) that the PID compares with its setpoint.
        // On():     called when the loop starts; nothing to do since the PWM always runs.
        // Off():    called when the loop is stopped; turns the motor off (duty 0).
        // Update(): receives the PID result each sample time and applies it as the PWM duty.
        float Get() { return current_rpm_; }
        void On() {}
        void Off() { SetOutput(0); }
        void Update(float output) { SetOutput((uint16_t)output); }

    private:
        static const uint8_t kPulsesPerRev = 3 * 4 * 4;             // feedback pulses per shaft revolution
        static const uint8_t kSamples = 10;                         // moving-average length
        static const uint32_t kMinPeriodUs = 600;              // edges closer than this are noise (> 1000 RPM)
        static const uint32_t kStallTimeoutUs = 500000;        // no edge for this long -> 0 RPM

        uint16_t current_rpm_ = 0;
        uint16_t set_rpm_ = 0;

        // Shared between the ISR and the main loop, hence volatile.
        volatile uint32_t last_edge_us_ = 0;     // micros() of the last accepted edge
        volatile uint32_t periods_[kSamples] = {}; // ring buffer of pulse periods [us]
        volatile uint8_t head_ = 0;                   // next slot to overwrite
        volatile uint8_t count_ = 0;                  // valid samples (<= kSamples)
};

#endif
