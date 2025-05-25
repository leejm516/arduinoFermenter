#include "MyFermenter.h"
#include "MAX31865.h"

void MyFermenter::onBaseControl(void) {
    lastBaseTime_ = millis();            
    basePumpOn_ = true;            
}

void MyFermenter::offBaseControl(void) {
    digitalWrite(basePumpPin_, LOW);
    basePumpOn_ = false;
    basePumpRunning_ = false;
}

void MyFermenter::runBasePump(double res) {
    if (basePumpOn_ == false) { return; }

    if (res >= 0.5) {        
        digitalWrite(basePumpPin_, LOW);
        basePumpRunning_ = false;
    } else {        
        
        if (basePumpRunning_ && nowTime_ - lastBaseTime_ >= baseOnInterval_) {
            basePumpRunning_ = !basePumpRunning_;
            digitalWrite(basePumpPin_, LOW);
            lastBaseTime_ = nowTime_;
        }

        if (!basePumpRunning_ && nowTime_ - lastBaseTime_ >= baseOffInterval_) {
            basePumpRunning_ = !basePumpRunning_;
            digitalWrite(basePumpPin_, HIGH);
            lastBaseTime_ = nowTime_;
        }
    }
}

void MyFermenter::setBaseOnInterval(unsigned long interval) {
    if (interval <= 300*1000) { baseOnInterval_ = interval; }
}

void MyFermenter::setBaseOffInterval(unsigned long interval) {
    if (interval <= 300*1000) {baseOffInterval_ = interval;}
}

void MyFermenter::readTempSensor(MAX31865_RTD temp) {
    if (temp.status() == 0) {
        currentTemp_ = temp.temperature();
    }
}

void MyFermenter::update(MAX31865_RTD temp) {
    nowTime_ = millis();

    currentPh_ = (7.0f + 0.1 * sin(2*PI*((float)nowTime_/1000/60/1.7) ) );
    // currentTemp_ = (37.0f + cos(2*PI*((float)nowTime_/1000/60/4) ) );
    temp.read_all();
    readTempSensor(temp);
    
    if (millis() % 1000 == 0) {
      Serial.print("현재의 pH는 ");
      Serial.println(this->getCurrentPh());

      Serial.print("현재의 온도는 ");
      Serial.print(this->getCurrentTemp());
      Serial.println("°C");
    }
}
