#ifndef SERVOCONTROL_H
#define SERVOCONTROL_H

#include <ESP32Servo.h>

class ServoControl {
public:
    ServoControl(int pin); // Constructeur pour initialiser le servo sur un pin spécifique
    void ouvrir();         // Fonction pour ouvrir (position 180°)
    void fermer();         // Fonction pour fermer (position 0°)
    void setAngle(int angle); // Fonction pour définir un angle spécifique

private:
    Servo servo;           // Objet Servo
    int servoPin;          // Pin utilisé pour le servo
};

#endif // SERVOCONTROL_H
