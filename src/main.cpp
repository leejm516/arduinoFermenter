#include <Arduino.h>
#include "SimplePhController.h"

// put function declarations here:
// int myFunction(int, int);

SimplePhController phControl(30, 31);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(2000);

  phControl.setOutput();
  phControl.setControlOn();

  while (true) {
    if ( millis() % 1000 == 0 ) {
        Serial.print("현재 pH: ");
        Serial.print(phControl.getCurrentPh());

        Serial.print(", 현재 state: ");
        Serial.print(phControl.getCurrentState());

        Serial.print(", 마지막 event: ");
        Serial.println(phControl.getLastEvent());
    }
    phControl.detectEvent();
    phControl.processState(millis());
  } 
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }