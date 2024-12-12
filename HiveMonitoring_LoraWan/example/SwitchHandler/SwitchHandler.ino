#include "SwitchHandler.h"

// Define the pin for the switch
#define SWITCH_PIN 1

// Create a SwitchHandler object
SwitchHandler switchHandler(SWITCH_PIN);

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(9600);

    // Initialize the switch handler
    switchHandler.begin();
}

void loop() {
    // Read the switch state
    bool isSwitchOn = switchHandler.readSwitch();

    // Print the switch state to the serial monitor
    if (isSwitchOn) {
        Serial.println("Switch is ON");
    } else {
        Serial.println("Switch is OFF");
    }

    // Add a small delay to avoid flooding the serial monitor
    delay(500);
}
