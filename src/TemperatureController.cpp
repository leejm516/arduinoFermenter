#include "TemperatureController.h"

void TemperatureController::init() {
    SPI.begin();
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE3));
    tempSensor.configure( true, true, false, false, MAX31865_FAULT_DETECTION_NONE,
                 true, false, 0x0000, 0x7fff ); // 2 wire RTD
}

void TemperatureController::update() {
    tempSensor.read_all();
    if (tempSensor.status() == 0) {
        currentTemp_ = tempSensor.temperature();
    }
}

float TemperatureController::getCurrentTemp() {    
    return currentTemp_;
}

float TemperatureController::getSetTemp() {    
    return setTemp_;
}