#include <Servo.h>

// Crée une instance de la classe Servo
Servo myServo;

// Définissez la broche sur laquelle vous branchez le fil de commande du servo
int servoPin = 13;  // Adapter si nécessaire

void setup() {
  // Attache le servo à la broche définie
  myServo.attach(servoPin);
  
  // Position initiale du servo (en degrés : 0 à 180)
  myServo.write(0);
  delay(1000);
}

void loop() {
  // Déplacement du servo de 0 à 180 degrés
  for (int pos = 0; pos <= 180; pos += 1) {
    myServo.write(pos);
    delay(15); // Temporisation pour que le servo ait le temps de se déplacer
  }

  // Déplacement du servo de 180 à 0 degrés
  for (int pos = 180; pos >= 0; pos -= 1) {
    myServo.write(pos);
    delay(15);
  }
}
