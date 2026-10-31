#ifndef _CONTROLLOOP_H
#define _CONTROLLOOP_H

#include <Arduino.h>
#include <PID_v1.h>

class RelayUpdate {
  public:
    virtual void On() = 0;
    virtual void Off() = 0;
    virtual void Update(float) = 0;
};

class DataSource {
  public:
    virtual float Get() = 0;
};



class ControlLoop {

  public:
    ControlLoop(DataSource*, DataSource*, RelayUpdate*, float);
    ControlLoop(DataSource* data, RelayUpdate* update, float setpoint) : ControlLoop(NULL, data, update, setpoint) {;};


    static const bool kPid_Debug = true;

    static const int kInner  = 0;
    static const int kOuter = 1;

    static const int kOnOff = 10;
    static const int kStd = 11;
    static const int kCascade = 12;

    bool Compute();

    void SetPoint(float);
    float GetSetPoint() { return set_point_; }
    float GetInnerSetPoint();



    void EnableBangBang() { bangbang_on_ = true; }
    void DisableBangBang() { bangbang_on_ = false; }
    bool IsBangBangOn() { return bangbang_on_;  }
    void SetBangBangRange(float x) { SetBangBangRange(-x, x); }
    void SetBangBangRange(float, float);
    float GetBangBangLower() { return bangbang_lower_; }
    float GetBangBangUpper() { return bangbang_upper_;}

    void SetControlType(int);
    bool IsControlOnOff() { return this->control_state_ == ControlLoop::kOnOff; }
    bool IsControlStandardPID() { return this->control_state_ == ControlLoop::kStd; }
    bool IsControlCascadePID() { return this->control_state_ == ControlLoop::kCascade; }
    int GetControlType(){ return this->control_state_; }


    void SetSampleTime(unsigned int);
    void SetOuterSampleFactor(unsigned int);

    bool IsOn() { return is_on_; }
    void SetOn() { SetOnOff(true); }
    void SetOff() { SetOnOff(false); }

    void SetOutputLimits(int, float, float);
    void SetDirectionIncrease(int, bool);
    bool GetDirectionIncrease(int);

    void SetTunings(float p, float i, float d) { SetTunings(ControlLoop::kInner, p, i, d);}
    void SetTunings(int, float, float, float);
    float GetKp(int);
    float GetKi(int);
    float GetKd(int);


  protected:

    void UpdateInputs();

    void SetOnOff(bool);

  private:

    float outer_in_ = 0.0;
    float outer_out_inner_set_ = 0.0;
    float outer_set_ = 0.0;
    // Setpoint is shared with inner Out

    float inner_in_ = 0.0;
    float inner_out_ = 0.0;

    float out_min_ = 0.0;
    float out_max_ = 1.0;


    PID pid_outer_;
    PID pid_inner_;

    PID *GetController_(int c);

    unsigned long sample_time_ms_ = 2500;
    unsigned int sample_factor_ = 4; // Recommended to be 3-5 times

    unsigned int control_state_ = kStd;

    float set_point_;
    float set_point_lower_;
    float set_point_upper_;

    bool is_on_ = false;
    bool bangbang_on_ = false;
    float bangbang_lower_ = 0.0;
    float bangbang_upper_ = 0.0;


    DataSource *inner_data_source_ = NULL;
    DataSource *outer_data_source_ = NULL;
    RelayUpdate *relay_ = NULL;

};

#endif

