#include <Arduino.h>
#include "SimplePhController.h"
#include "Relay.h"
#include "ControlLoop.h"
#include "TemperatureController.h"
#include "FermenterData.h"

// put function declarations here:
// int myFunction(int, int);

SimplePhController phControl(30, 31);

// For temp control using Relay.h and ControlLoop.h
TemperatureController tempCon(46);

Relay heaterRelay(32, 2);
Relay coolerRelay(33, 2);

// Structs for communication with ESP32
PvProfile pvProfile;

class : public DataSource{
  public:
    double get() {
      return tempCon.getCurrentTemp();
    }
} tempDataSource;

class : public RelayUpdate {
  public:
    void on() {
      heaterRelay.setRelayMode(relayModeAutomatic);
    }
    void off() {
      heaterRelay.setRelayMode(relayModeManual);
      heaterRelay.setDutyCyclePercent(0.0);
    }
    void update(double res) {
      heaterRelay.setDutyCyclePercent(res);
      heaterRelay.loop();
    }
} heater;

class : public RelayUpdate {
  public:
    void on() {
      coolerRelay.setRelayMode(relayModeAutomatic);
    }
    void off() {
      coolerRelay.setRelayMode(relayModeManual);
      coolerRelay.setDutyCyclePercent(0.0);
    }
    void update(double res) {
      coolerRelay.setDutyCyclePercent(res);
      coolerRelay.loop();
    }
} cooler;

ControlLoop heaterControlLoop(&tempDataSource, &heater, tempCon.getSetTemp());
ControlLoop coolerControlLoop(&tempDataSource, &cooler, tempCon.getSetTemp());


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(2000);

    // pH control
  phControl.setOutput();
  phControl.setControlOn();

  tempCon.init(MAX31865_2WIRE);

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
    pvProfile.temp = tempCon.getCurrentTemp();
    pvProfile.ph = phControl.getCurrentPh();

    // print some data
    if ( ct - pt >= 1000 ) {
        pt += 1000;
        Serial.print("현재 pH: ");
        Serial.print(phControl.getCurrentPh());

        Serial.print(", 현재 state: ");
        Serial.print(phControl.getCurrentState());

        Serial.print(", 마지막 event: ");
        Serial.print(phControl.getLastEvent());

        Serial.print(", 현재 온도:");
        Serial.print(tempCon.getCurrentTemp());
        Serial.println("°C");
    }   


    phControl.detectEvent();
    phControl.processState(ct);

    tempCon.update();
    coolerControlLoop.Compute();
    heaterControlLoop.Compute();
  } 
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }