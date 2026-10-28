#include "bldcmotor.h"

// Timer4 channel A is wired to D6 on the Mega, so the PWM pin is fixed.
static const uint8_t kPwmPin = 6;

void BldcMotor::Begin(uint8_t signal_pin) {
    // The feedback line is pulled up so it doesn't float if the driver is unplugged.
    pinMode(signal_pin, INPUT_PULLUP);
    pinMode(kPwmPin, OUTPUT);

    // Timer4 fast PWM (mode 14), TOP = ICR4, prescaler 8
    // -> 16 MHz / 8 / (999 + 1) = 2 kHz. Timer4 does not affect millis()/micros() (Timer0).
    TCCR4A = _BV(COM4A1) | _BV(WGM41);
    TCCR4B = _BV(WGM43) | _BV(WGM42) | _BV(CS41);
    ICR4 = kMaxOutput;
    OCR4A = 0;   // start with the motor stopped
}

void BldcMotor::SetOutput(uint16_t output) {
    OCR4A = min(output, kMaxOutput);   // OCR4A is the duty: 0 = off, kMaxOutput = full
}

// Runs in interrupt context (interrupts already disabled), so keep it short.
void BldcMotor::OnEdge() {
    unsigned long now = micros();
    unsigned long period = now - last_edge_us_;   // unsigned subtraction is micros() overflow-safe

    // Debounce: an edge sooner than any real pulse can arrive is noise.
    // Don't touch last_edge_us_, so the next real edge is still measured from the last good one.
    if (period < kMinPeriodUs) return;
    last_edge_us_ = now;

    // First edge after a stop (or after boot): the period is meaningless,
    // so discard old samples and start averaging from scratch.
    if (period >= kStallTimeoutUs) {
        count_ = 0;
        head_ = 0;
        return;
    }

    // Store the period in the ring buffer, overwriting the oldest sample once full.
    periods_[head_] = period;
    head_ = (head_ + 1) % kSamples;
    if (count_ < kSamples) count_++;
}

void BldcMotor::CalculateRpm() {
    unsigned long sum = 0;
    uint8_t n;
    unsigned long last_edge;

    // Take a consistent snapshot: the ISR may modify the buffer at any time.
    // SREG is restored rather than calling sei() so this is safe regardless of the caller's interrupt state.
    uint8_t sreg = SREG;
    cli();
    for (uint8_t i = 0; i < count_; i++) sum += periods_[i];
    n = count_;
    last_edge = last_edge_us_;
    SREG = sreg;

    // No samples yet, or no pulse for a while (motor stopped/stalled):
    // report 0 instead of holding the last, stale value.
    if (n == 0 || micros() - last_edge >= kStallTimeoutUs) {
        current_rpm_ = 0;
        return;
    }

    // mean period [us] -> pulse frequency [Hz] -> revolutions per second -> RPM (+0.5 to round)
    float mean_period_us = (float)sum / n;
    current_rpm_ = (uint16_t)(1000000.0f / mean_period_us / kPulsesPerRev * 60.0f + 0.5f);
}
