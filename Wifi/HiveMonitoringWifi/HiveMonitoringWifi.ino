#include <ESP32Servo.h>
#include "Seeed_vl53l0x.h"
#include "LIS3DHTR.h"
#include "AHT20.h"
#include "SwitchHandler.h"
#include <math.h>
#include <CayenneLPP.h>

// Déclarations globales
#define SEUIL_FERME 100         // Distance en mm pour considérer "fermé"
#define SEUIL_OUVERT 500        // Distance en mm pour considérer "ouvert"
#define TEMP_FERMETURE 24.0     // Température en °C pour fermer la porte
#define TEMP_OUVERTURE 23.7     // Température en °C pour ouvrir la porte
#define SWITCH_PIN 1
#define SERVO_PIN 2

// Objets capteurs et servo
Seeed_vl53l0x VL53L0X;
AHT20 AHT;
Servo monServo;
SwitchHandler switchHandler(SWITCH_PIN);

// Variables globales
int distance = 0;
int angleActuel = 0;
float humidity = 0, temperature = 0;
bool enOuverture = false;
bool enFermeture = false;
bool controleAutomatique = true; // Active le contrôle automatique par température
String positionDemandee = "Fermé";
const char* descriptionDistance = "";
CayenneLPP lpp(51); // Taille du buffer pour le payload
int dernierEtatMouvement = 0;
int etatDistance = 0; 

CayenneLPP dataToSend(CayenneLPP lpp) {
    lpp.reset();

    afficherDonnees(); // Afficher toutes les données avant envoi pour debug

    lpp.addDigitalInput(1, dernierEtatMouvement); // État du mouvement
    lpp.addDigitalInput(2, etatDistance);        // État de la distance
    lpp.addTemperature(3, temperature);         // Température
    lpp.addRelativeHumidity(4, humidity * 100); // Humidité relative

    /*Serial.print("CayenneLPP Payload: ");
    for (size_t i = 0; i < lpp.getSize(); i++) {
        Serial.print(lpp.getBuffer()[i], HEX);
        Serial.print(" ");
    }
    Serial.println();*/

    return lpp;
}

// Fonction pour retourner l'angle du servo
int getAngle() {
    return angleActuel;
}

// Initialisation
void setup() {
    Serial.begin(115200);
    while (!Serial);

    // Initialisation du capteur de distance
    if (VL53L0X.VL53L0X_common_init() != VL53L0X_ERROR_NONE) {
        Serial.println("Erreur d'initialisation du VL53L0X!");
        while (1);
    }
    VL53L0X.VL53L0X_continuous_ranging_init();

    // Initialisation du switch et du servo
    pinMode(SWITCH_PIN, INPUT_PULLUP);
    switchHandler.begin();
    monServo.attach(SERVO_PIN);
    monServo.write(angleActuel);

    // Initialisation du capteur AHT20
    AHT.begin(); // Initialiser le capteur
    // Optionnel : vérifier si la température ou l'humidité renvoie une valeur raisonnable
    float testHumidity, testTemperature;
    if (!AHT.getSensor(&testHumidity, &testTemperature)) {
        Serial.println("Erreur d'initialisation du capteur AHT20!");
        while (1);
    } else {
        Serial.println("AHT20 initialisé avec succès.");
    }

}

// Ouvrir le servo
void ouvrir() {
    if (angleActuel + 5 <= 180) {
        angleActuel += 5;
        monServo.write(angleActuel);
        delay(15);
    }
}

// Fermer le servo
void fermer() {
    if (angleActuel - 5 >= 0) {
        angleActuel -= 5;
        monServo.write(angleActuel);
        delay(15);
    }
}

// Gestion de l'ouverture
void gererOuverture() {
    if (distance >= SEUIL_OUVERT) {
        enOuverture = false;
        Serial.println("Position ouverte atteinte.");
    } else {
        ouvrir();
    }
}

// Gestion de la fermeture
void gererFermeture() {
    if (distance <= SEUIL_FERME) {
        enFermeture = false;
        Serial.println("Position fermée atteinte.");
    } else {
        fermer();
    }
}

// Lecture des capteurs
void mesurerDistance() {
    VL53L0X_RangingMeasurementData_t RangingMeasurementData;
    VL53L0X.PerformContinuousRangingMeasurement(&RangingMeasurementData);
    if (RangingMeasurementData.RangeMilliMeter > 0 && RangingMeasurementData.RangeMilliMeter < 2000) {
        distance = RangingMeasurementData.RangeMilliMeter;
        descriptionDistance = (distance <= SEUIL_FERME) ? "fermé" :
                              (distance >= SEUIL_OUVERT) ? "ouvert" : "en mouvement";
    } else {
        descriptionDistance = "hors portée";
    }
}

void mesurerTemperatureHumidite() {
    if (!AHT.getSensor(&humidity, &temperature)) {
        Serial.println("Erreur de lecture du capteur AHT20!");
    }
}

// Contrôle automatique par température
void controleParTemperature() {
    if (temperature >= TEMP_FERMETURE && positionDemandee != "Fermé") {
        positionDemandee = "Fermé";
        enOuverture = false;
        enFermeture = true;
        Serial.println("Température élevée, fermeture automatique de la porte.");
    } else if (temperature <= TEMP_OUVERTURE && positionDemandee != "Ouvert") {
        positionDemandee = "Ouvert";
        enOuverture = true;
        enFermeture = false;
        Serial.println("Température basse, ouverture automatique de la porte.");
    }
}

// Affichage des données
void afficherDonnees() {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" mm | Position demandée: ");
    Serial.print(positionDemandee);
    Serial.print(" | État actuel: ");
    Serial.print(descriptionDistance);
    Serial.print(" | Angle moteur: ");
    Serial.print(getAngle());
    Serial.print(" | Température: ");
    Serial.print(temperature);
    Serial.print(" °C | Humidité: ");
    Serial.print(humidity * 100);
    Serial.println(" %");
}

// Boucle principale
void loop() {
    // Lecture de l'état du switch
    bool switchState = switchHandler.readSwitch();

    // Si le switch est activé, désactiver le contrôle automatique et basculer manuellement
    if (switchState) {
        controleAutomatique = false; // Désactive le contrôle automatique
        if (positionDemandee == "Ouvert") {
            positionDemandee = "Fermé";
            enFermeture = true;
            enOuverture = false;
            Serial.println("Switch activé : fermeture manuelle.");
        } else {
            positionDemandee = "Ouvert";
            enOuverture = true;
            enFermeture = false;
            Serial.println("Switch activé : ouverture manuelle.");
        }
    }

    // Lecture des capteurs
    mesurerDistance();
    mesurerTemperatureHumidite();

    // Contrôle automatique si activé
    if (controleAutomatique) {
        controleParTemperature();
    }

    // Gestion de l'ouverture et de la fermeture
    if (enOuverture) {
        gererOuverture();
    }
    if (enFermeture) {
        gererFermeture();
    }

    // Afficher les données dans le terminal
    //afficherDonnees();

    // Envoi périodique du payload CayenneLPP
    dataToSend(lpp);

    delay(50);
}
