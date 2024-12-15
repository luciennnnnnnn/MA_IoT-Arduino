#include <Servo.h>
#include "Seeed_vl53l0x.h"

// Création de l'objet Servo
Servo monServo;

// Variables de position et d'état du servomoteur
int angleActuel = 0;
const int pas = 5; // Pas de mouvement en degrés

// Configuration des broches
const int servoPin = 13;  // Broche pour le servo
const int switchPin = 15;  // Broche pour le switch (interrupteur)

// États pour gérer le switch et les modes
bool dernierSwitchState = false;
bool enOuverture = false;
bool enFermeture = false;

// Configuration pour le capteur de distance
Seeed_vl53l0x VL53L0X;
const int SEUIL_FERME = 100; // Distance en mm pour considérer "fermé"
const int SEUIL_OUVERT = 500; // Distance en mm pour considérer "ouvert"
int distance = 0;

// Position demandée
String positionDemandee = "Fermé"; // Par défaut, "Fermé"

// Fonction pour obtenir l'angle actuel
int getAngle() {
  return angleActuel;
}

void setup() {
  Serial.begin(115200); // Communication série pour le debug

  // Configuration des broches
  pinMode(switchPin, INPUT_PULLUP); // Configure le switch en entrée avec pull-up

  // Initialisation du servomoteur
  monServo.attach(servoPin);
  monServo.write(angleActuel);

  // Initialisation du capteur de distance
  if (VL53L0X.VL53L0X_common_init() != VL53L0X_ERROR_NONE) {
    Serial.println("Erreur lors de l'initialisation du VL53L0X!");
    while (1);
  }
  VL53L0X.VL53L0X_continuous_ranging_init();

  Serial.println("Système prêt.");
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
  // Lecture de la distance
  VL53L0X_RangingMeasurementData_t RangingMeasurementData;
  VL53L0X.PerformContinuousRangingMeasurement(&RangingMeasurementData);
  distance = RangingMeasurementData.RangeMilliMeter;

  // Lecture de l'état du switch
  bool lectureSwitch = digitalRead(switchPin);

  // Détection de flanc montant pour alterner entre ouverture et fermeture
  if (lectureSwitch && !dernierSwitchState) {
    if (positionDemandee == "Ouvert") {
      positionDemandee = "Fermé"; // Passe à "Fermé"
      enOuverture = false;
      enFermeture = true;
      Serial.println("Passage en mode fermeture.");
    } else {
      positionDemandee = "Ouvert"; // Passe à "Ouvert"
      enFermeture = false;
      enOuverture = true;
      Serial.println("Passage en mode ouverture.");
    }
  }
  dernierSwitchState = lectureSwitch;

  // Gestion de l'ouverture
  if (enOuverture) {
    if (distance >= SEUIL_OUVERT) { // Si la distance atteint le seuil d'ouverture
      enOuverture = false;
      Serial.println("Position ouverte atteinte.");
    } else {
      ouvrir();
    }
  }

  // Gestion de la fermeture
  if (enFermeture) {
    if (distance <= SEUIL_FERME) { // Si la distance atteint le seuil de fermeture
      enFermeture = false;
      Serial.println("Position fermée atteinte.");
    } else {
      fermer();
    }
  }

  // Affichage des informations
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

  delay(50); // Délai pour lisser les lectures
}
