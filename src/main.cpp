#include <Arduino.h>
#include "simplephcontroller.h"
#include "relay.h"
#include "controlloop.h"
#include "temperaturecontroller.h"
#include "fermenterdata.h"

// put function declarations here:


// Initialization of objects and structures
SimplePhController ph_control(30, 31);

// For temp control using Relay.h and ControlLoop.h
TemperatureController temp_controller(46);

Relay heater_relay(32, 2);
Relay cooler_relay(33, 2);

// Structs for communication with ESP32
PvProfile pv_profile;


// Anonymous classes to use controlloop.h
class : public DataSource{
  public:
    double get() {
      return temp_controller.getCurrentTemp();
    }
} temp_data_source;

class : public RelayUpdate {
  public:
    void on() {
      heater_relay.setRelayMode(kRelayModeAutomatic);
    }
    void off() {
      heater_relay.setRelayMode(kRelayModeManual);
      heater_relay.setDutyCyclePercent(0.0);
    }
    void update(double res) {
      heater_relay.setDutyCyclePercent(res);
      heater_relay.loop();
    }
} heater;

class : public RelayUpdate {
  public:
    void on() {
      cooler_relay.setRelayMode(kRelayModeAutomatic);
    }
    void off() {
      cooler_relay.setRelayMode(kRelayModeManual);
      cooler_relay.setDutyCyclePercent(0.0);
    }
    void update(double res) {
      cooler_relay.setDutyCyclePercent(res);
      cooler_relay.loop();
    }
} cooler;

ControlLoop heaterControlLoop(&temp_data_source, &heater, temp_controller.getSetTemp());
ControlLoop coolerControlLoop(&temp_data_source, &cooler, temp_controller.getSetTemp());


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(2000);

    // pH control
  ph_control.setOutput();
  ph_control.setControlOn();

  temp_controller.init(MAX31865_2WIRE);

  delay(100);

  // Temp control
  // cooler loop
  coolerControlLoop.setControlType(ControlLoop::STD);
  coolerControlLoop.setBangBangRange(0.5, 5);
  coolerControlLoop.enableBangBang();
  coolerControlLoop.setOutputLimits(ControlLoop::INNER, 0.0, 1.0);
  coolerControlLoop.setTunings(0.05, 0.005, 0.001);
  coolerControlLoop.setDirectionIncrease(ControlLoop::INNER, 0); // pid 제어에서만 작동함
  coolerControlLoop.setOn();  
  
  // heater loop
  heaterControlLoop.setControlType(ControlLoop::STD);
  heaterControlLoop.setBangBangRange(5, 0.5);
  heaterControlLoop.enableBangBang();
  heaterControlLoop.setOutputLimits(ControlLoop::INNER, 0.0, 1.0);
  heaterControlLoop.setTunings(0.05, 0.005, 0.001);
  heaterControlLoop.setDirectionIncrease(ControlLoop::INNER, 1); // pid 제어에서만 작동함
  heaterControlLoop.setOn();  

  uint32_t pt = millis();
  
  // Start of the main loop
  while (true) {    
    unsigned long ct = millis();

    // Save measured profile into a PvProfile struct
    pv_profile.temp = temp_controller.getCurrentTemp();
    pv_profile.ph = ph_control.getCurrentPh();

    // print some data
    if ( ct - pt >= 1000 ) {
        pt += 1000;
        Serial.print("현재 pH: ");
        Serial.print(ph_control.getCurrentPh());

        Serial.print(", 현재 state: ");
        Serial.print(ph_control.getCurrentState());

        Serial.print(", 마지막 event: ");
        Serial.print(ph_control.getLastEvent());

        Serial.print(", 현재 온도:");
        Serial.print(temp_controller.getCurrentTemp());
        Serial.println("°C");
    }   


    ph_control.detectEvent();
    ph_control.processState(ct);

    temp_controller.update();
    coolerControlLoop.Compute();
    heaterControlLoop.Compute();
  } 
}

void loop() {
}

