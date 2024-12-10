#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Servo.h>

class ServoControl {
private:
    int servoPin;          // Pin du servo
    int currentAngle;      // Angle actuel du servo (0 à 180)
    Servo servo;           // Objet Servo pour contrôler le moteur

public:
    // Constructeur pour initialiser le servo sur le pin donné
    ServoControl(int pin);

    // Fonction pour ouvrir le servo (position 180°)
    void openTo180();

    // Fonction pour fermer le servo (position 0°)
    void closeTo0();

    // Fonction pour définir un angle spécifique
    void setAngle(int angle);

    // Fonction pour démarrer l'ouverture progressivement
    void startOpening();

    // Fonction pour arrêter l'ouverture (maintenir l'angle actuel)
    void stopOpening();

    // Fonction pour démarrer la fermeture progressivement
    void startClosing();

    // Fonction pour arrêter la fermeture (maintenir l'angle actuel)
    void stopClosing();
};

#endif
