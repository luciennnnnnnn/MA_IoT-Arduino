#ifndef SWITCH_HANDLER_H
#define SWITCH_HANDLER_H

#include <Arduino.h>

// Class to handle switch reading
class SwitchHandler {
public:
    // Constructor
    SwitchHandler(int pin);

    // Initialize the pin as input
    void begin();

    // Read the switch state
    bool readSwitch();

private:
    int switchPin; // Pin connected to the switch
};

#endif
