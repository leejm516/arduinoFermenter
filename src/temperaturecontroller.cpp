#include "temperaturecontroller.h"

void TemperatureController::init(max31865_numwires_t numwire) {
    tempSensor.begin(numwire);
    tempSensor.setOffset(0.24f);
}

void TemperatureController::update() {
    uint8_t fault = tempSensor.readFault();    
    if (fault == 0) {
        current_temp_ = tempSensor.temperature();
    }
    // tempSensor.read_all();
    // if (tempSensor.status() == 0) {
    //     currentTemp_ = tempSensor.temperature();
    // }
}

float TemperatureController::getCurrentTemp() {    
    return current_temp_;
}

float TemperatureController::getSetTemp() {    
    return set_temp_;
}

void TemperatureController::setTemp(float sp) {
    set_temp_ = sp;
}