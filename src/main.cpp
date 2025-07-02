#include <Arduino.h>
#include "simplephcontroller.h"
#include "relay.h"
#include "controlloop.h"
#include "temperaturecontroller.h"
#include "fermenterdata.h"
#include "bldcmotor.h"

// put function declarations here:


// Initialization of objects and structures
SimplePhController ph_control(30, 31);

// Structs for communication with ESP32
PvProfile pv_profile;

// For temp control using Relay.h and ControlLoop.h
TemperatureController temp_controller(46);

Relay heater_relay(32, 2);
Relay cooler_relay(33, 2);

// Anonymous classes to use controlloop.h
class : public DataSource{
  public:
    float Get() {
      return temp_controller.getCurrentTemp();
    }
} temp_data_source;

class : public RelayUpdate {
  public:
    void On() {
      heater_relay.SetRelayMode(kRelayModeAutomatic);
    }
    void Off() {
      heater_relay.SetRelayMode(kRelayModeManual);
      heater_relay.SetDutyCyclePercent(0.0);
    }
    void Update(float res) {
      heater_relay.SetDutyCyclePercent(res);
      heater_relay.Loop();
    }
} heater;

class : public RelayUpdate {
  public:
    void On() {
      cooler_relay.SetRelayMode(kRelayModeAutomatic);
    }
    void Off() {
      cooler_relay.SetRelayMode(kRelayModeManual);
      cooler_relay.SetDutyCyclePercent(0.0);
    }
    void Update(float res) {
      cooler_relay.SetDutyCyclePercent(res);
      cooler_relay.Loop();
    }
} cooler;

ControlLoop heater_control_loop(&temp_data_source, &heater, temp_controller.getSetTemp());
ControlLoop cooler_control_loop(&temp_data_source, &cooler, temp_controller.getSetTemp());


// motor control
const uint8_t signal_pin = 2;
const uint8_t pwm_pin = 6;
BldcMotor motor(pwm_pin, signal_pin);

void MeasureMotorPeriod(void);

class : public DataSource{
  public:
    float Get() {
      return motor.GetCurrentRpm();
    }
} motor_data_source;

class : public RelayUpdate {
  public:
    void On() {      
    }
    void Off() {
      motor.SetOutput(0);      
    }
    void Update(float res) {
      motor.SetOutput((unsigned int)res);      
    }
} motor_relay;

ControlLoop motor_control_loop(&motor_data_source, &motor_relay, 0);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(2000);

    // pH control
  ph_control.SetOutput();
  ph_control.SetControlOn();

  temp_controller.init(MAX31865_2WIRE);

  Serial.println("온도센서 초기화...");
  delay(100);


  // Motor control for agitation
  // pwm_pin: PWM signal for controlling motor driver, D6 pin (arduino mega)
  // signal_pin: feedback signal from a motor driver
  pinMode(signal_pin, INPUT_PULLUP);
  pinMode(pwm_pin, OUTPUT);

  // Modify PWN mode on pin D6 by directly setting registers
  // Fast PWM, 2 kHz
  TCCR4A = _BV(COM4A1) | _BV(WGM41);
  TCCR4B = _BV(WGM43) | _BV(WGM42) | _BV(CS41);

  ICR4 = 999;
  OCR4A = 0;

  attachInterrupt(digitalPinToInterrupt(signal_pin), MeasureMotorPeriod, RISING);

  Serial.println("모터 구동 설정...");
  delay(100);

  // Temp control
  // cooler loop
  cooler_control_loop.SetControlType(ControlLoop::kStd);
  cooler_control_loop.SetBangBangRange(0.5, 5);
  cooler_control_loop.EnableBangBang();
  cooler_control_loop.SetOutputLimits(ControlLoop::kInner, 0.0, 1.0);
  cooler_control_loop.SetTunings(0.05, 0.005, 0.001);
  cooler_control_loop.SetDirectionIncrease(ControlLoop::kInner, 0); // pid 제어에서만 작동함
  cooler_control_loop.SetOn();  
  
  // heater loop
  heater_control_loop.SetControlType(ControlLoop::kStd);
  heater_control_loop.SetBangBangRange(5, 0.5);
  heater_control_loop.EnableBangBang();
  heater_control_loop.SetOutputLimits(ControlLoop::kInner, 0.0, 1.0);
  heater_control_loop.SetTunings(0.05, 0.005, 0.001);
  heater_control_loop.SetDirectionIncrease(ControlLoop::kInner, 1); // pid 제어에서만 작동함
  heater_control_loop.SetOn();  

  // motor loop
  motor_control_loop.SetControlType(ControlLoop::kStd);
  motor_control_loop.SetOutputLimits(ControlLoop::kInner, 0.0, 999.0);
  motor_control_loop.SetTunings(0.5, 0.5, 0.075);
  motor_control_loop.SetSampleTime(250);
  motor_control_loop.SetOn();  

  unsigned long pt = millis();
  
  // Start of the main loop
  while (true) {    
    unsigned long ct = millis();

    // Save measured profile into a PvProfile struct
    pv_profile.temp = temp_controller.getCurrentTemp();
    pv_profile.ph = ph_control.GetCurrentPh();

    // print some data
    if ( ct - pt >= 1000 ) {
        pt += 1000;
        Serial.print("현재 pH: ");
        Serial.print(ph_control.GetCurrentPh());

        Serial.print(", 현재 state: ");
        Serial.print(ph_control.GetCurrentState());

        Serial.print(", 마지막 event: ");
        Serial.print(ph_control.GetLastEvent());

        Serial.print(", 현재 온도:");
        Serial.print(temp_controller.getCurrentTemp());
        Serial.println("°C");       

        Serial.print(", 현재 RPM: ");
        Serial.println(motor.GetCurrentRpm());

    }   

    ph_control.DetectEvent();
    ph_control.ProcessState(ct);

    temp_controller.update();
    cooler_control_loop.Compute();
    heater_control_loop.Compute();    

    motor.CalculateRpm();
    motor_control_loop.Compute();

    if (Serial.available()) {      
      String input_string = Serial.readStringUntil('\n');
      uint16_t input_rpm = input_string.toInt();
      Serial.print("RPM이 입력되었습니다: ");
      Serial.println(input_string);
      motor.SetRpm(input_rpm);
      motor_control_loop.SetPoint(motor.GetSetRpm());
    }
  } 
}

void loop() {
}

void MeasureMotorPeriod() {
  static volatile unsigned long last_update = 0;
  static volatile unsigned long last_time = 0;
  unsigned long current_time = micros();
  if (current_time - last_update >= 2000) {
    if (last_time > 0) {
        unsigned long period = current_time - last_time;
        motor.Update(period);
    }
    last_update = current_time;
  }
  last_time = current_time;
}
