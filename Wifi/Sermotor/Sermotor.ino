#include <ESP32Servo.h>
#include "Seeed_vl53l0x.h"

Servo monServo; // Création de l'objet Servo
int angleActuel = 0; // Position initiale du servomoteur
const int pas = 5; // Pas d'incrémentation en degrés
const int switchPin = 1; // Pin du switch
bool dernierSwitchState = false; // Dernier état du switch
bool enOuverture = false; // Indique si le moteur est en mode ouverture
bool enFermeture = false; // Indique si le moteur est en mode fermeture

// Configuration pour le capteur de distance
Seeed_vl53l0x VL53L0X;
const int SEUIL_FERME = 100; // Distance en mm pour considérer "fermé"
const int SEUIL_OUVERT = 500; // Distance en mm pour considérer "ouvert"
int distance = 0;

String positionDemandee = "Fermé"; // Par défaut, position demandée à "Fermé"

// Fonction pour retourner l'angle actuel du moteur
int getAngle() {
  return angleActuel;
}

void setup() {
  Serial.begin(115200); // Initialisation de la communication série pour le debug
  pinMode(switchPin, INPUT_PULLUP); // Configure le switch en entrée avec pull-up
  monServo.attach(2); // Attache le servomoteur à la broche 2 (GPIO 2 sur ESP32)
  monServo.write(angleActuel); // Positionne le servo à l'angle initial

  // Initialisation du capteur de distance
  if (VL53L0X.VL53L0X_common_init() != VL53L0X_ERROR_NONE) {
    Serial.println("Erreur lors de l'initialisation du VL53L0X!");
    while (1);
  }
  VL53L0X.VL53L0X_continuous_ranging_init();
}

void ouvrir() {
  if (angleActuel + pas <= 180) { // Vérifie si l'angle reste dans la plage
    angleActuel += pas;
    monServo.write(angleActuel);
    Serial.print("Ouverture: angle actuel = ");
    Serial.println(angleActuel);
    delay(15); // Donne au servo le temps de se déplacer
  }
}

void fermer() {
  if (angleActuel - pas >= 0) { // Vérifie si l'angle reste dans la plage
    angleActuel -= pas;
    monServo.write(angleActuel);
    Serial.print("Fermeture: angle actuel = ");
    Serial.println(angleActuel);
    delay(15); // Donne au servo le temps de se déplacer
  }
}

void loop() {
  // Lecture constante de la distance
  VL53L0X_RangingMeasurementData_t RangingMeasurementData;
  VL53L0X.PerformContinuousRangingMeasurement(&RangingMeasurementData);
  distance = RangingMeasurementData.RangeMilliMeter;

  // Lecture de l'état du switch
  bool lectureSwitch = digitalRead(switchPin);

  // Détection de flanc montant pour alterner entre ouverture et fermeture
  if (lectureSwitch && !dernierSwitchState) {
    if (positionDemandee == "Ouvert") {
      positionDemandee = "Fermé"; // Demande de fermeture
      enOuverture = false;
      enFermeture = true;
      Serial.println("Passage en mode fermeture.");
    } else {
      positionDemandee = "Ouvert"; // Demande d'ouverture
      enFermeture = false;
      enOuverture = true;
      Serial.println("Passage en mode ouverture.");
    }
  }
  dernierSwitchState = lectureSwitch;

  // Gestion de l'ouverture
  if (enOuverture) {
    if (distance >= SEUIL_OUVERT) { // Si la distance indique "ouvert"
      enOuverture = false; // Arrête l'ouverture
      Serial.println("Position ouverte atteinte.");
    } else {
      ouvrir(); // Continue à ouvrir
    }
  }

  // Gestion de la fermeture
  if (enFermeture) {
    if (distance <= SEUIL_FERME) { // Si la distance indique "fermé"
      enFermeture = false; // Arrête la fermeture
      Serial.println("Position fermée atteinte.");
    } else {
      fermer(); // Continue à fermer
    }
  }

  // Affichage constant des données de distance, des états, et de l'angle
  String etatActuel = (distance <= SEUIL_FERME) ? "Fermé" : ((distance >= SEUIL_OUVERT) ? "Ouvert" : "En Mouvement");
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" mm | ");
  Serial.print("Position demandée: ");
  Serial.print(positionDemandee);
  Serial.print(" | ");
  Serial.print("État actuel: ");
  Serial.print(etatActuel);
  Serial.print(" | ");
  Serial.print("Angle moteur: ");
  Serial.println(getAngle());

  delay(50); // Petit délai pour éviter de saturer les lectures
}
