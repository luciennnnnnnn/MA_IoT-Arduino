#include <Wire.h>
#include <Seeed_vl53l0x.h>
#include "ServoControl.h"

// Définir les pins
const int servoPin = 2; // Pin du servomoteur
const int distanceSeuil = 100; // Distance seuil en mm

// Définir les constantes
#define DISTANCE_SEUIL 50 // Distance seuil en mm
#define TIMEOUT 5000      // Timeout en ms

// Créer une instance du servomoteur
ServoControl myServo(servoPin);

// Initialiser le capteur de distance
Seeed_vl53l0x distanceSensor;

// Variables pour la gestion du timeout
unsigned long startTime = 0;
bool isServoActive = false;

void setup() {
    Serial.begin(115200);

    // Initialiser le servomoteur
    myServo.fermer(); // Assurez-vous que le moteur commence fermé
    Serial.println("Servo initialisé.");

    // Initialiser le capteur de distance
    if (distanceSensor.VL53L0X_common_init() != VL53L0X_ERROR_NONE) {
        Serial.println("Erreur d'initialisation du capteur de distance !");
        while (1);
    }
    Serial.println("Capteur de distance initialisé.");
}

void loop() {
    // Lire la distance du capteur
    VL53L0X_RangingMeasurementData_t RangingMeasurementData;
    distanceSensor.PerformContinuousRangingMeasurement(&RangingMeasurementData);

    if (RangingMeasurementData.RangeMilliMeter >= 2000) {
        Serial.println("Distance hors de portée.");
        return;
    }

    int distance = RangingMeasurementData.RangeMilliMeter;
    Serial.print("Distance mesurée : ");
    Serial.print(distance);
    Serial.println(" mm");

    // Démarrer le servo si une condition est remplie (par exemple, distance > seuil haut)
    if (!isServoActive && distance > distanceSeuil) {
        myServo.ouvrir();
        isServoActive = true;
        startTime = millis(); // Démarrer le timer
        Serial.println("Servo activé (ouverture).");
    }

    // Arrêter le servo si la distance atteint le seuil bas ou timeout
    if (isServoActive) {
        if (distance <= distanceSeuil || millis() - startTime > TIMEOUT) {
            myServo.fermer();
            isServoActive = false;
            Serial.println("Servo désactivé (fermeture).");
        }
    }

    delay(100); // Petit délai pour éviter les lectures trop rapides
}
