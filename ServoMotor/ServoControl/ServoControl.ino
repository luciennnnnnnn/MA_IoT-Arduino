#include <Wire.h>
#include <Seeed_vl53l0x.h>
#include "ServoControl.h"

// Définir les pins
const int servoPin = 2; // Pin du servomoteur
const int distanceSeuil = 100; // Distance seuil en mm

// Créer une instance du servomoteur
ServoControl myServo(servoPin);

// Initialiser le capteur de distance
Seeed_vl53l0x distanceSensor;

// Variable pour suivre l'état du servo
bool isServoActive = false;

void setup() {
    Serial.begin(115200);

    // Initialiser le servomoteur
    myServo.ouvrir(); // Assurez-vous que le moteur commence ouvert
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

    // Démarrer le servo si la distance dépasse le seuil
    if (!isServoActive && distance > distanceSeuil + 20) {
        myServo.ouvrir();
        isServoActive = true;
        Serial.println("Servo activé (ouverture).");
    }

    // Fermer le servo si la distance est inférieure ou égale au seuil
    if (isServoActive && distance <= distanceSeuil) {
        myServo.fermer();
        isServoActive = false;
        Serial.println("Servo désactivé (fermeture).");
    }

    delay(100); // Petit délai pour éviter les lectures trop rapides
}
