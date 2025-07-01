#ifndef _BLDCMOTOR_H
#define _BLDCMOTOR_H

#include <Arduino.h>

class BldcMotor {
    public:
        BldcMotor() : pwm_pin_(6), 
                      signal_pin_(11), 
                      current_rpm_(0), 
                      output_(0),
                      set_rpm_(0),                      
                      kp_(5.0), 
                      ki_(0.1), 
                      kd_(0.0),
                      last_time_(0),
                      periods_{0,0,0,0,0} {
        }

        BldcMotor(uint8_t pwm_pin, uint8_t signal_pin) : pwm_pin_(pwm_pin), 
                                                         signal_pin_(signal_pin), 
                                                         current_rpm_(0), 
                                                         output_(0),
                                                         set_rpm_(0),                                                         
                                                         kp_(5.0), 
                                                         ki_(0.1), 
                                                         kd_(0.0),
                                                         last_time_(0),
                                                         periods_{0,0,0,0,0} {
        }

        void update(unsigned long);

        unsigned int getCurrentRpm(void);

        unsigned int getSetRpm(void);

        void setRpm(unsigned int);

        void setOutput(unsigned int);

        unsigned long getLastTime(void);

        void setLastTime(unsigned long);

        unsigned long calculateMeanPeriod(void);

        void calculateRpm(void);

    private:
        uint8_t pwm_pin_;
        uint8_t signal_pin_;
        uint16_t current_rpm_;
        uint16_t output_;
        uint16_t set_rpm_;
        float kp_;
        float ki_;
        float kd_;
        volatile unsigned long last_time_; // last time for period aquisition
        volatile unsigned long periods_[5];
};

#endif