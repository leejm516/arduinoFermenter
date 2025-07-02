#include "controlloop.h"

//#define _SHOW_COMPUTE_TRACE(...) Serial.println(__VA_ARGS__ )
#define _SHOW_COMPUTE_TRACE(...) 



ControlLoop::ControlLoop(DataSource* in_ds, DataSource* ot_ds, RelayUpdate* r, float sp) :
  pid_outer_(&outer_in_, &outer_out_inner_set_, &outer_set_, 1.0, 0.0, 0.0, DIRECT),
  pid_inner_(&inner_in_, &inner_out_, &outer_out_inner_set_, 1.0, 0.0, 0.0, DIRECT) {

  inner_data_source_ = in_ds;
  outer_data_source_ = ot_ds;
  relay_ = r;
  set_point_ = sp;

  out_min_ = 0.0;
  out_max_ = 1.0;

  pid_inner_.SetOutputLimits(out_min_, out_max_);
  pid_outer_.SetOutputLimits(0.0, 90.0);

  is_on_ = false;
  pid_inner_.SetMode(MANUAL);
  pid_outer_.SetMode(MANUAL);

  pid_inner_.SetSampleTime(sample_time_ms_);
  pid_outer_.SetSampleTime(sample_time_ms_*sample_factor_);

  SetControlType(kStd);
  UpdateInputs();
}


PID* ControlLoop::GetController_(int c) {
  if ( c == ControlLoop::kOuter ) {
    return &pid_outer_;
  }
  return &pid_inner_;
}

void ControlLoop::UpdateInputs() {
  switch (this->control_state_) {
    case ControlLoop::kCascade:
      this->outer_in_ = outer_data_source_->Get();
      // Handle case if kCascade is set without a second data source. 
      if (inner_data_source_ == NULL) {
        this->inner_in_ = this->outer_in_;
      } else {
        this->inner_in_ = inner_data_source_->Get();
      }
      break;
      
    case ControlLoop::kStd:
    case ControlLoop::kOnOff:
      this->outer_in_ = outer_data_source_->Get();
      this->inner_in_ = this->outer_in_;
      break;

    default:
      Serial.print(F("ERROR: unknown control type "));
      Serial.println(this->control_state_);    
  }
}

bool ControlLoop::Compute() {

  bool updated = false;

  UpdateInputs();

  _SHOW_COMPUTE_TRACE("Control: %i bb: %i sp: %f, %f %f ", this->control_state_, bangbang_on_, set_point_, set_point_lower_, set_point_upper_);

  // Jump out as off.
  if (! is_on_) {
    _SHOW_COMPUTE_TRACE("not on, returning\n");
    return false;
  }


  if ( this->control_state_ == ControlLoop::kOnOff ) {
    _SHOW_COMPUTE_TRACE("onoff ");

    if ( bangbang_on_ ) {
      _SHOW_COMPUTE_TRACE("bb ");
      updated = true;

      if ( inner_in_ < set_point_lower_ ) {
        _SHOW_COMPUTE_TRACE("max ");
        inner_out_ = this->pid_inner_.GetDirection() ? out_min_ : out_max_;

      } else if ( inner_in_ > set_point_upper_ ) {
        _SHOW_COMPUTE_TRACE("min ");
        inner_out_ = this->pid_inner_.GetDirection() ? out_max_ : out_min_;

      } else {
        _SHOW_COMPUTE_TRACE("n/a ");

        // No else to handle bewteen setpts
        // will continue previous setting
        // e.g. rising will continue
        // failing will continue
        ;

      }

    } else {
      _SHOW_COMPUTE_TRACE("!bb ");

      updated = true;
      if ( inner_in_ < set_point_ ) {
        _SHOW_COMPUTE_TRACE("max ");
        inner_out_ = out_max_;

      } else if ( inner_in_ > set_point_ ) {
        _SHOW_COMPUTE_TRACE("min ");
        inner_out_ = out_min_;

      } else {
        // State is equal, keep doing previous.
        _SHOW_COMPUTE_TRACE("n/a ");
        ;
      }

    }



  } else if ( this->control_state_ == ControlLoop::kStd ) {
    _SHOW_COMPUTE_TRACE("std ");

    if ( bangbang_on_ && inner_in_ < set_point_lower_ ) {
      _SHOW_COMPUTE_TRACE("bb/low ");
      updated = true;
      inner_out_ = this->pid_inner_.GetDirection() ? out_min_ : out_max_;

    } else if ( bangbang_on_ && inner_in_ > set_point_upper_ ) {
      _SHOW_COMPUTE_TRACE("bb/high ");
      updated = true;
      inner_out_ = this->pid_inner_.GetDirection() ? out_max_ : out_min_;

    } else {
      _SHOW_COMPUTE_TRACE("compute ");
      updated = pid_inner_.Compute();
    }

  } else if ( this->control_state_ == ControlLoop::kCascade ) {
    _SHOW_COMPUTE_TRACE("cas ");
    if ( bangbang_on_ && inner_in_ < set_point_lower_ ) {
      _SHOW_COMPUTE_TRACE("bb/low ");
      updated = true;
      inner_out_ = out_max_;

    } else if ( bangbang_on_ && inner_in_ > set_point_upper_ ) {
      _SHOW_COMPUTE_TRACE("bb/high ");
      updated = true;
      inner_out_ = out_min_;

    } else {
      _SHOW_COMPUTE_TRACE("compute ");
      bool o = pid_outer_.Compute();
      bool i = pid_inner_.Compute();
      updated = i || o;
    }

  } else {
    Serial.print(F("ERROR: unknown control type "));
    Serial.println(this->control_state_);
    updated = false;

  }

  if ( updated ) {
    relay_->Update(inner_out_);
  }

  _SHOW_COMPUTE_TRACE("upd rt: %i\n", updated);
  return updated;
}

void ControlLoop::SetControlType(int ct) {

  this->control_state_ = ct;

  if ( this->control_state_ == ControlLoop::kOnOff ) {
    pid_inner_.SetMode(MANUAL);
    pid_outer_.SetMode(MANUAL);

  } else if ( this->control_state_ == ControlLoop::kStd ) {
    pid_inner_.SetMode(AUTOMATIC);
    pid_outer_.SetMode(MANUAL);
    outer_out_inner_set_ = set_point_;

  } else if ( this->control_state_ == ControlLoop::kCascade ) {
    pid_inner_.SetMode(AUTOMATIC);
    pid_outer_.SetMode(AUTOMATIC);
    outer_set_ = set_point_;
    outer_out_inner_set_ = set_point_;

  } else {
    Serial.print(F("ERROR: unknown control type "));
    Serial.println(this->control_state_);
  }

}

void ControlLoop::SetPoint(float sp) {

  set_point_ = sp;
  set_point_lower_ = sp - bangbang_lower_;
  set_point_upper_ = sp + bangbang_upper_;
  SetControlType(control_state_);
}


float ControlLoop::GetInnerSetPoint() {
  if (this->control_state_ == kCascade ) {
    return outer_out_inner_set_;
  }
  return -1.0;
}

void ControlLoop::SetBangBangRange(float lower, float upper) {
  if ( lower > 0 && upper > 0 ) {
    bangbang_lower_ = lower;
    bangbang_upper_ = upper;
    SetPoint(set_point_);
  }
}



void ControlLoop::SetSampleTime(unsigned int ms) {
  sample_time_ms_ = ms;
  pid_inner_.SetSampleTime(sample_time_ms_);
  pid_outer_.SetSampleTime(sample_time_ms_*sample_factor_);
}

void ControlLoop::SetOuterSampleFactor(unsigned int factor) {
  sample_factor_ = factor;
  pid_outer_.SetSampleTime(sample_time_ms_*sample_factor_);
}


void ControlLoop::SetOnOff(bool turn_on) {

  pid_inner_.SetMode(MANUAL);
  pid_outer_.SetMode(MANUAL);
  is_on_ = turn_on;

  if ( is_on_ ) {
    relay_->On();
    if ( this->control_state_ == ControlLoop::kOnOff ) {
      ;
    } else if ( this->control_state_ == ControlLoop::kStd ) {
      pid_inner_.SetMode(AUTOMATIC);

    } else if ( this->control_state_ == ControlLoop::kCascade ) {
      pid_inner_.SetMode(AUTOMATIC);
      pid_outer_.SetMode(AUTOMATIC);

    } else {
      Serial.print(F("ERROR: unknown control type "));
      Serial.println(this->control_state_);
    }
  } else {
    relay_->Off();
  }


}


void ControlLoop::SetOutputLimits(int  c, float _min, float _max) {
  PID* thePid = this->GetController_(c);
  thePid->SetOutputLimits(_min, _max);

  if ( c == ControlLoop::kInner ) {
    out_min_ = _min;
    out_max_ = _max;
  }

}


void ControlLoop::SetDirectionIncrease(int  c, bool dir) {
  PID *thePid = this->GetController_(c);
  if ( dir ) {
    thePid->SetControllerDirection(DIRECT);
  } else {
    thePid->SetControllerDirection(REVERSE);
  }
}

bool ControlLoop::GetDirectionIncrease(int  c) {
  PID *thePid = this->GetController_(c);
  return thePid->GetDirection() == DIRECT;
}

void ControlLoop::SetTunings(int  c, float p, float i, float d) {
  PID *thePid = this->GetController_(c);
  thePid->SetTunings(p, i, d);
}

float ControlLoop::GetKp(int  c) {
  PID *thePid = this->GetController_(c);
  return thePid->GetKp();
}

float ControlLoop::GetKi(int  c) {
  PID *thePid = this->GetController_(c);
  return thePid->GetKi();
}

float ControlLoop::GetKd(int  c) {
  PID *thePid = this->GetController_(c);
  return thePid->GetKd();
}

