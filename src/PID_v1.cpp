/**********************************************************************************************
 * Arduino PID Library - Version 1.2.1
 * by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
 *
 * This Library is licensed under the MIT License
 **********************************************************************************************/

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <PID_v1.h>

/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
PID::PID(float* input, float* output, float* set_point,
        float kp, float ki, float kd, int p_on, int controller_direction)
{
    my_output_ = output;
    my_input_ = input;
    my_setpoint_ = set_point;
    in_auto_ = false;

    PID::SetOutputLimits(0, 255);				//default output limit corresponds to
												//the arduino pwm limits

    sample_time_ = 100;							//default Controller Sample Time is 0.1 seconds

    PID::SetControllerDirection(controller_direction);
    PID::SetTunings(kp, ki, kd, p_on);

    last_time_ = millis() - sample_time_;
}

/*Constructor (...)*********************************************************
 *    To allow backwards compatability for v1.1, or for people that just want
 *    to use Proportional on Error without explicitly saying so
 ***************************************************************************/

PID::PID(float* input, float* output, float* set_point,
        float kp, float ki, float kd, int controller_direction)
    :PID::PID(input, output, set_point, kp, ki, kd, P_ON_E, controller_direction)
{

}


/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/
bool PID::Compute()
{
   if ( !in_auto_ ) return false;
   
   unsigned long now = millis();
   unsigned long time_change = (now - last_time_);
   if ( time_change >= sample_time_ ) {
      /*Compute all the working error variables*/
      float input = *my_input_;
      float error = *my_setpoint_ - input;
      float d_input = (input - last_input_);
      output_sum_ += (ki_ * error);

      /*Add Proportional on Measurement, if P_ON_M is specified*/
      if (!p_on_e_) output_sum_ -= kp_ * d_input;

      if (output_sum_ > out_max_) output_sum_ = out_max_;
      else if (output_sum_ < out_min_) output_sum_ = out_min_;

      /*Add Proportional on Error, if P_ON_E is specified*/
	   float output;
      if(p_on_e_) output = kp_ * error;
      else output = 0;

      /*Compute Rest of PID Output*/
      output += output_sum_ - kd_ * d_input;

      if (output > out_max_) {
         output = out_max_;
      } else if (output < out_min_) {
         output = out_min_;
      }
      
      *my_output_ = output;
      

      /*Remember some variables for next time*/
      last_input_ = input;
      last_time_ = now;
	   return true;

   }  else return false;
}

/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void PID::SetTunings(float kp, float ki, float kd, int p_on)
{
   if (kp < 0 || ki < 0 || kd < 0) return;

   p_on_ = p_on;
   p_on_e_ = p_on == P_ON_E;

   disp_kp_ = kp; disp_ki_ = ki; disp_kd_ = kd;

   float sample_time_in_sec = ((float)sample_time_)/1000;
   kp_ = kp;
   ki_ = ki * sample_time_in_sec;
   kd_ = kd / sample_time_in_sec;

  if(controller_direction_ == REVERSE)
   {
      kp_ = (0 - kp_);
      ki_ = (0 - ki_);
      kd_ = (0 - kd_);
   }
}

/* SetTunings(...)*************************************************************
 * Set Tunings using the last-rembered POn setting
 ******************************************************************************/
void PID::SetTunings(float kp, float ki, float kd){
    SetTunings(kp, ki, kd, p_on_e_); 
}

/* SetSampleTime(...) *********************************************************
 * sets the period, in Milliseconds, at which the calculation is performed
 ******************************************************************************/
void PID::SetSampleTime(int new_sample_time)
{
   if (new_sample_time > 0)
   {
      float ratio  = (float)new_sample_time
                      / (float)sample_time_;
      ki_ *= ratio;
      kd_ /= ratio;
      sample_time_ = (unsigned long)new_sample_time;
   }
}

/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 **************************************************************************/
void PID::SetOutputLimits(float min_val, float max_val)
{
   if (min_val >= max_val) return;
   out_min_ = min_val;
   out_max_ = max_val;

   if (in_auto_) {
	   if (*my_output_ > out_max_ ) *my_output_ = out_max_;
	   else if(*my_output_ < out_min_) *my_output_ = out_min_;

	   if(output_sum_ > out_max_) output_sum_= out_max_;
	   else if(output_sum_ < out_max_) output_sum_= out_min_;
   }
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 ******************************************************************************/
void PID::SetMode(int mode)
{
    bool new_auto = (mode == AUTOMATIC);
    if (new_auto && !in_auto_)
    {  /*we just went from manual to auto*/
        PID::Initialize();
    }
    in_auto_ = new_auto;
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void PID::Initialize()
{
   output_sum_ = *my_output_;
   last_input_ = *my_input_;
   if (output_sum_ > out_max_) output_sum_ = out_max_;
   else if(output_sum_ < out_min_) output_sum_ = out_min_;
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void PID::SetControllerDirection(int direction)
{
   if(in_auto_ && direction != controller_direction_) {
	   kp_ = (0 - kp_);
      ki_ = (0 - ki_);
      kd_ = (0 - kd_);
   }
   controller_direction_ = direction;
}

/* Status Funcions*************************************************************
 * Just because you set the Kp=-1 doesn't mean it actually happened.  these
 * functions query the internal state of the PID.  they're here for display
 * purposes.  this are the functions the PID Front-end uses for example
 ******************************************************************************/
float PID::GetKp(){ return  disp_kp_; }
float PID::GetKi(){ return  disp_ki_;}
float PID::GetKd(){ return  disp_kd_;}
int PID::GetMode(){ return  in_auto_ ? AUTOMATIC : MANUAL;}
int PID::GetDirection(){ return controller_direction_;}

