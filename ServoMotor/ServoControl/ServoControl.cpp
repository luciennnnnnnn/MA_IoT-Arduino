#include "ServoControl.h"

// Constructeur pour initialiser le servo sur le pin donné
ServoControl::ServoControl(int pin) {
    servoPin = pin;
    servo.attach(servoPin); // Attacher le servo au pin
}

// Fonction pour ouvrir le servo (position 180°)
void ServoControl::ouvrir() {
    servo.write(180); // Position maximale (ouverte)
}

// Fonction pour fermer le servo (position 0°)
void ServoControl::fermer() {
    servo.write(0); // Position minimale (fermée)
}

// Fonction pour définir un angle spécifique
void ServoControl::setAngle(int angle) {
    if (angle >= 0 && angle <= 180) {
        servo.write(angle);
    }
}
