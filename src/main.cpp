#include <Arduino.h>
#include <SPI.h>
#include "Relay.h"
#include "ControlLoop.h"
#include "MyFermenter.h"
#include "MAX31865.h"

MAX31865_RTD tempSensor(MAX31865_RTD::RTD_PT100, 36);

MyFermenter fermenter1;

Relay coolRelay(fermenter1.getCoolerPin(), 4);
Relay heatRelay(fermenter1.getHeaterPin(), 4);

class : public DataSource {
  public:
    double get() {
      return fermenter1.getCurrentPh();
    }
} phDS1;

class : public RelayUpdate {
  public:
    void on() {      
      fermenter1.onBaseControl();
      ;
    }
    void off() {
      fermenter1.offBaseControl();
      // myRelay.setRelayMode(relayModeManual);
      // myRelay.setDutyCyclePercent(0.0);
    }
    void update(double res) {
      fermenter1.runBasePump(res);
      // myRelay.setDutyCyclePercent(res);
      // myRelay.loop();
    }
} basePump;

ControlLoop baseControlLoop(&phDS1, &basePump, 7.05);


class : public DataSource {
  public:
    double get() {
      return fermenter1.getCurrentTemp();
    }
} tempDS;

class : public DataSource {
  public:
    double get() {
      return fermenter1.getCurrentTemp();
    }
} tempDS2;

class : public RelayUpdate {
  public:
    void on() {
      coolRelay.setRelayMode(relayModeAutomatic);
    }
    void off() {
      coolRelay.setRelayMode(relayModeManual);
      coolRelay.setDutyCyclePercent(0.0);
    }
    void update(double res) {
      coolRelay.setDutyCyclePercent(res);
      coolRelay.loop();
    }
} coolerRelay;

class : public RelayUpdate {
  public:
    void on() {
      heatRelay.setRelayMode(relayModeAutomatic);
    }
    void off() {
      heatRelay.setRelayMode(relayModeManual);
      heatRelay.setDutyCyclePercent(0.0);
    }
    void update(double res) {
      heatRelay.setDutyCyclePercent(res);
      heatRelay.loop();
    }
} heaterRelay;


ControlLoop coolControlLoop(&tempDS, &coolerRelay, 37);
ControlLoop heatControlLoop(&tempDS2, &heaterRelay, 37);

// put function declarations here:
// int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(2000);

  // pinMode(48, OUTPUT);

  SPI.begin();
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE3));
  delay(1000);
  tempSensor.configure( true, true, false, false, MAX31865_FAULT_DETECTION_NONE,
                 true, false, 0x0000, 0x7fff );
                 
  Serial.println("기동");
  Serial.print("HeaterPin 번호는: ");
  Serial.println(fermenter1.getHeaterPin());

  baseControlLoop.setControlType(ControlLoop::ONOFF);
  baseControlLoop.setOn();

  // cooler loop 설정
  coolControlLoop.setControlType(ControlLoop::STD);
  coolControlLoop.setBangBangRange(0.5, 5);
  coolControlLoop.enableBangBang();
  coolControlLoop.setOutputLimits(ControlLoop::INNER, 0.0, 1.0);
  coolControlLoop.setTunings(0.05, 0.005, 0.001);
  coolControlLoop.setDirectionIncrease(ControlLoop::INNER, 0); // pid 제어에서만 작동함
  coolControlLoop.setOn();  
  
  // heater loop 설정
  heatControlLoop.setControlType(ControlLoop::STD);
  heatControlLoop.setBangBangRange(5, 0.5);
  heatControlLoop.enableBangBang();
  heatControlLoop.setOutputLimits(ControlLoop::INNER, 0.0, 1.0);
  heatControlLoop.setTunings(0.05, 0.005, 0.001);
  // heatControlLoop.setDirectionIncrease(ControlLoop::INNER, 1); // pid 제어에서만 작동함
  heatControlLoop.setOn();  

  

  while (true) {        
    fermenter1.update(tempSensor);
    baseControlLoop.Compute();
    coolControlLoop.Compute();
    heatControlLoop.Compute();
  } 
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }