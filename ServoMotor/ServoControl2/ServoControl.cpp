#include "ServoControl.h"
#include <Arduino.h>

// Constructeur pour initialiser le servo sur le pin donné
ServoControl::ServoControl(int pin) {
    servoPin = pin;
    servo.attach(servoPin); // Attacher le servo au pin
    currentAngle = 0; // Initialiser l'angle actuel à 0° (porte fermée)
    servo.write(currentAngle); // Positionner le servo à l'angle initial
}

// Fonction pour ouvrir le servo (position 180°)
void ServoControl::openTo180() {
    currentAngle = 180;  // Mettre l'angle à la position ouverte
    servo.write(currentAngle);  // Déplacer le servo vers 180°
}

// Fonction pour fermer le servo (position 0°)
void ServoControl::closeTo0() {
    currentAngle = 0;  // Mettre l'angle à la position fermée
    servo.write(currentAngle);  // Déplacer le servo vers 0°
}

// Fonction pour définir un angle spécifique
void ServoControl::setAngle(int angle) {
    if (angle >= 0 && angle <= 180) {
        currentAngle = angle;  // Mettre à jour l'angle actuel
        servo.write(currentAngle);  // Déplacer le servo à l'angle spécifié
    }
}

// Fonction pour démarrer l'ouverture progressivement
void ServoControl::startOpening() {
    if (currentAngle < 180) {
        currentAngle++;  // Incrémenter l'angle progressivement
        servo.write(currentAngle);  // Déplacer le servo vers la nouvelle position
    }
}

// Fonction pour arrêter l'ouverture (maintenir l'angle actuel)
void ServoControl::stopOpening() {
    // Rien à faire ici, car l'angle est déjà maintenu à sa position actuelle
}

// Fonction pour démarrer la fermeture progressivement
void ServoControl::startClosing() {
    if (currentAngle > 0) {
        currentAngle--;  // Décrémenter l'angle progressivement
        servo.write(currentAngle);  // Déplacer le servo vers la nouvelle position
    }
}

// Fonction pour arrêter la fermeture (maintenir l'angle actuel)
void ServoControl::stopClosing() {
    // Rien à faire ici, car l'angle est déjà maintenu à sa position actuelle
}
