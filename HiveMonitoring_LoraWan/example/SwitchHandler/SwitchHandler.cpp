#include "SwitchHandler.h"

// Constructor to initialize the pin
SwitchHandler::SwitchHandler(int pin) : switchPin(pin) {}

// Set up the pin mode
void SwitchHandler::begin() {
    pinMode(switchPin, INPUT);
}

// Read and return the state of the switch
bool SwitchHandler::readSwitch() {
    return digitalRead(switchPin) == HIGH;
}
