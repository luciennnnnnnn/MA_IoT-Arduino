#include <ESP32Servo.h>
#include "Seeed_vl53l0x.h"
#include "LIS3DHTR.h"
#include "AHT20.h"
#include "SwitchHandler.h"
#include <CayenneLPP.h>
#include <math.h>

// Déclarations globales
#define SEUIL_FERME 100         // Distance en mm pour considérer "fermé"
#define SEUIL_OUVERT 500        // Distance en mm pour considérer "ouvert"
#define SEUIL_NORME 0.3         // Seuil pour la variation de norme vectorielle
#define TEMPORISATION_STATIQUE 5 // Cycles pour confirmer l'état statique
#define SWITCH_PIN 1
#define SERVO_PIN 2

// Objets capteurs et servo
Seeed_vl53l0x VL53L0X;
LIS3DHTR<TwoWire> LIS;
AHT20 AHT;
Servo monServo;
SwitchHandler switchHandler(SWITCH_PIN);

// Variables globales
float humidity = 0, temperature = 0;
float ax = 0, ay = 0, az = 0;
float lastNorme = 0.0;
int compteurStatique = 0;
int dernierEtatMouvement = 1;
int distance = 0;
int angleActuel = 0;
bool enOuverture = false;
bool enFermeture = false;
String positionDemandee = "Fermé";
const char* descriptionMouvement = "";
const char* descriptionDistance = "";

// Fonction pour calculer la norme vectorielle
float calculerNorme(float x, float y, float z) {
    return sqrt(x * x + y * y + z * z);
}

// Fonction pour retourner l'angle du servo
int getAngle() {
    return angleActuel;
}

// Initialisation
void setup() {
    Serial.begin(115200);
    while (!Serial);

    // Initialisation des capteurs
    if (VL53L0X.VL53L0X_common_init() != VL53L0X_ERROR_NONE) {
        Serial.println("Erreur d'initialisation du VL53L0X!");
        while (1);
    }
    VL53L0X.VL53L0X_continuous_ranging_init();

    Wire.begin();
    LIS.begin(Wire, LIS3DHTR_ADDRESS_UPDATED);
    if (!LIS.isConnection()) {
        Serial.println("Erreur d'initialisation du LIS3DHTR!");
        while (1);
    }
    LIS.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);

    AHT.begin();

    // Initialisation du switch et du servo
    pinMode(SWITCH_PIN, INPUT_PULLUP);
    switchHandler.begin();
    monServo.attach(SERVO_PIN);
    monServo.write(angleActuel);
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

void mesurerAccelerometre() {
    ax = LIS.getAccelerationX();
    ay = LIS.getAccelerationY();
    az = LIS.getAccelerationZ();

    float normeActuelle = calculerNorme(ax, ay, az);
    float variationNorme = fabs(lastNorme - normeActuelle);
    lastNorme = normeActuelle;

    if (variationNorme > SEUIL_NORME) {
        dernierEtatMouvement = 2;
        compteurStatique = 0;
    } else {
        compteurStatique++;
        if (compteurStatique >= TEMPORISATION_STATIQUE) {
            dernierEtatMouvement = 1;
        }
    }
    descriptionMouvement = (dernierEtatMouvement == 2) ? "en mouvement" : "statique";
}

void mesurerTemperatureHumidite() {
    if (!AHT.getSensor(&humidity, &temperature)) {
        Serial.println("Erreur de lecture du capteur AHT20!");
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
    Serial.print(" | Mouvement: ");
    Serial.print(descriptionMouvement);
    Serial.print(" | Température: ");
    Serial.print(temperature);
    Serial.print(" °C | Humidité: ");
    Serial.print(humidity * 100);
    Serial.println(" %");
}

// Boucle principale
void loop() {
    bool switchState = switchHandler.readSwitch();

    if (switchState) {
        if (positionDemandee == "Ouvert") {
            positionDemandee = "Fermé";
            enFermeture = true;
            enOuverture = false;
        } else {
            positionDemandee = "Ouvert";
            enOuverture = true;
            enFermeture = false;
        }
    }

    mesurerDistance();
    mesurerAccelerometre();
    mesurerTemperatureHumidite();

    if (enOuverture) {
        gererOuverture();
    }

    if (enFermeture) {
        gererFermeture();
    }

    afficherDonnees();
    delay(50);
}
