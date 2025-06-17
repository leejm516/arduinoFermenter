#include "TemperatureController.h"

void TemperatureController::init(max31865_numwires_t numwire) {
    tempSensor.begin(numwire);
}

void TemperatureController::update() {
    uint8_t fault = tempSensor.readFault();    
    if (fault == 0) {
        currentTemp_ = tempSensor.temperature();
    }
    // tempSensor.read_all();
    // if (tempSensor.status() == 0) {
    //     currentTemp_ = tempSensor.temperature();
    // }
}

float TemperatureController::getCurrentTemp() {    
    return currentTemp_;
}

float TemperatureController::getSetTemp() {    
    return setTemp_;
}