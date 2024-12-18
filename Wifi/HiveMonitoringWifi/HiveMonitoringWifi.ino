#include <ESP32Servo.h>
#include "Seeed_vl53l0x.h"
#include "LIS3DHTR.h"
#include "AHT20.h"
#include "SwitchHandler.h"
#include <math.h>
#include <CayenneLPP.h>
#include "aws_config.h"

// === Configuration et constantes ===
#undef SERIAL
#define SERIAL Serial

// Seuils et configurations de capteurs et moteur
#define SEUIL_FERME 100         // Distance pour "fermé" en mm
#define SEUIL_OUVERT 500        // Distance pour "ouvert" en mm
#define SEUIL_NORME 0.3         // Seuil de variation de la norme vectorielle
#define TEMPORISATION_STATIQUE 5 // Cycles pour confirmer l'état statique
#define TEMP_FERMETURE 25.0     // Température pour fermer la porte
#define TEMP_OUVERTURE 24.7     // Température pour ouvrir la porte
#define SWITCH_PIN 1
#define SERVO_PIN 2
#define BUTTON_B_PIN 0 // Remplacez 0 par le GPIO correspondant au bouton B


// === Variables globales ===
bool lastButtonBState = HIGH; // État précédent du bouton B (non pressé)
bool currentButtonBState = HIGH; // État actuel du bouton B
unsigned long lastDebounceTime = 0; // Temps de la dernière lecture stable
unsigned long debounceDelay = 50; // Temps de rebond (en ms)
float humidity = 0, temperature = 0;
int etatDistance = 0;
float lastNorme = 0.0;
int compteurStatique = 0;
int dernierEtatMouvement = 1;
float ax = 0, ay = 0, az = 0;
int distance = 0;
int angleActuel = 0;
const char* descriptionDistance = "";
const char* descriptionSwitch = "";
const char* descriptionMode = "";
const char* descriptionMouvement = "";
String positionDemandee = "Fermé";
bool enOuverture = false;
bool enFermeture = false;
bool controleAutomatique = true; // Active le contrôle automatique par température
bool controleManuel = true; // Active le contrôle automatique par switch
unsigned long previousMillis = 0; // Dernière fois que la fonction a été exécutée
unsigned long interval = 10000; // Intervalle en millisecondes (startTime en secondes)
float latitude = 0;
float longitude = 0;
float altitude = 0;


// === Objets capteurs et moteurs ===
Seeed_vl53l0x VL53L0X;
LIS3DHTR<TwoWire> LIS;
#define WIRE Wire
SwitchHandler switchHandler(SWITCH_PIN);
AHT20 AHT;
Servo monServo;

// === Fonctions auxiliaires ===

/**
 * @brief Calcule la norme vectorielle d'un vecteur 3D
 */
float calculerNorme(float x, float y, float z) {
    return sqrt(x * x + y * y + z * z);
}

/**
 * @brief Calcule une moyenne pondérée entre deux valeurs
 */
float calculerMoyenne(float ancienneValeur, float nouvelleValeur, float facteur) {
    return (ancienneValeur * (1.0 - facteur)) + (nouvelleValeur * facteur);
}

void getGps()
{
  latitude = 46.5233972;
  longitude = 6.6097495;
  altitude = 495;
}

// === Affichage des données ===
void printAllData() {
    SERIAL.print("Distance: ");
    SERIAL.print(distance);
    SERIAL.print(" mm | ");

    SERIAL.print("Position demandée: ");
    SERIAL.print(positionDemandee);
    SERIAL.print(" | ");

    SERIAL.print("État actuel: ");
    SERIAL.print(descriptionDistance);
    SERIAL.print(" | ");

    SERIAL.print("Angle moteur: ");
    SERIAL.print(angleActuel);
    SERIAL.print(" | ");

    SERIAL.print("Température: ");
    SERIAL.print(temperature);
    SERIAL.print(" °C | ");

    SERIAL.print("Humidité: ");
    SERIAL.print(humidity * 100);
    SERIAL.print(" % | ");

    SERIAL.print("Accéléromètre: X");
    SERIAL.print(ax, 2);
    SERIAL.print(" Y");
    SERIAL.print(ay, 2);
    SERIAL.print(" Z");
    SERIAL.print(az, 2);
    SERIAL.print(" | ");

    SERIAL.print("Mouvement: ");
    SERIAL.print(descriptionMouvement);
    SERIAL.println();

    descriptionMode = controleAutomatique ? "auto" : "manuel";

    SERIAL.print("\nMode: ");
    SERIAL.print(descriptionMode);
    SERIAL.println();
}

/**
 * @brief Envoi via MQTT
 */
// Function to prepare data to send
StaticJsonDocument<256> dataToSend() {
    // Create a JSON document to hold data
    StaticJsonDocument<256> doc;
    
    // Fill in the data
    printAllData();
    doc["is_moving"] = dernierEtatMouvement;
    doc["door"] = etatDistance;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity * 100;
    doc["device_id"] = ThingName;
    doc["latitude"] = latitude;
    doc["longitude"] = longitude;

    // Return the filled document
    return doc;
}

// Function to send data
void sendData(const StaticJsonDocument<256>& doc) {
    // Serialize the JSON document to a payload string
    char payload[256];
    serializeJson(doc, payload, sizeof(payload));

    // Publish the payload to the MQTT topic
    client.publish(publishTopic, payload);
    Serial.println("Données publiées : ");
    Serial.println(payload); // Print payload for debugging
}

// === Contrôle du servo moteur ===

/**
 * @brief Ouvre la porte en augmentant progressivement l'angle du servo
 */
void ouvrir() {
    if (angleActuel + 5 <= 180) {
        angleActuel += 5;
        monServo.write(angleActuel);
        delay(15);
    }
}

/**
 * @brief Ferme la porte en diminuant progressivement l'angle du servo
 */
void fermer() {
    if (angleActuel - 5 >= 0) {
        angleActuel -= 5;
        monServo.write(angleActuel);
        delay(15);
    }
}

/**
 * @brief Gère les actions lorsque la ruche est en mode OFF.
 */
void gererModeOFF() {
    // Arrêter les modes manuel et automatique
    controleManuel = false;
    controleAutomatique = false;

    // Mettre à jour la position demandée à "Fermé"
    if (positionDemandee != "Fermé") {
        positionDemandee = "Fermé";
        Serial.println("Ruche en mode OFF : fermeture demandée.");
    }

    // Forcer la fermeture de la porte
    if (!enFermeture) {
        enFermeture = true;
        enOuverture = false;
        Serial.println("Ruche en mode OFF : fermeture en cours.");
    }

    // Fermer la porte si ce n'est pas encore fait
    gererFermeture();
}


/**
 * @brief Gère l'ouverture automatique en fonction de la distance
 */
void gererOuverture() {
    if (distance >= SEUIL_OUVERT) {
        enOuverture = false;
        Serial.println("Position ouverte atteinte.");
    } else {
        ouvrir();
    }
}

/**
 * @brief Gère la fermeture automatique en fonction de la distance
 */
void gererFermeture() {
    if (distance <= SEUIL_FERME) {
        enFermeture = false;
        Serial.println("Position fermée atteinte.");
    } else {
        fermer();
    }
}

/**
 * @brief Active l'ouverture/fermeture avec le switch
 */
void controleParSwitch(){
  // Lire l'état du switch
  bool switchState = switchHandler.readSwitch();
  descriptionSwitch = switchState ? "pressé" : "relaché";

  if(switchState) {
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
}

/**
 * @brief Active l'ouverture/fermeture selon la température mesurée
 */
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

// === Lecture des capteurs ===

/**
 * @brief Lit et met à jour les données du capteur de distance
 */
void mesurerDistance() {
    VL53L0X_RangingMeasurementData_t RangingMeasurementData;
    VL53L0X.PerformContinuousRangingMeasurement(&RangingMeasurementData);

    if (RangingMeasurementData.RangeMilliMeter > 0 && RangingMeasurementData.RangeMilliMeter < 2000) {
        distance = RangingMeasurementData.RangeMilliMeter;

        if (distance <= SEUIL_FERME) {
            etatDistance = 1;
            descriptionDistance = "fermé";
        } else if (distance >= SEUIL_OUVERT) {
            etatDistance = 3;
            descriptionDistance = "ouvert";
        } else {
            etatDistance = 2;
            descriptionDistance = "en mouvement";
        }
    } else {
        distance = -1;
        etatDistance = 3;
        descriptionDistance = "hors portée";
        SERIAL.println("Distance: Hors de portée");
    }
}

/**
 * @brief Lit et met à jour les données de température et d'humidité
 */
void mesurerTemperatureHumidite() {
    if (!AHT.getSensor(&humidity, &temperature)) {
        Serial.println("Erreur de lecture du capteur AHT20!");
    }
}

/**
 * @brief Lit et met à jour les données du mouvement de la ruche
 */
void mesurerRucheEnMouvement() {
  // ----- Capteur d'accélération -----
    if (!LIS.isConnection()) {
        SERIAL.println("LIS3DHTR disconnected!");
        while (1);
    }

    // Lire les valeurs de l'accéléromètre
    ax = LIS.getAccelerationX();
    ay = LIS.getAccelerationY();
    az = LIS.getAccelerationZ();

    // Calculer la norme vectorielle et lisser les variations
    float normeActuelle = calculerNorme(ax, ay, az);
    float normeLisse = calculerMoyenne(lastNorme, normeActuelle, 0.1); // Facteur de lissage 0.1
    lastNorme = normeLisse;

    // Déterminer si la variation de la norme dépasse le seuil
    float variationNorme = fabs(normeLisse - normeActuelle);
    int nouvelEtatMouvement = (variationNorme > SEUIL_NORME) ? 2 : 1;

    // Appliquer une temporisation pour confirmer l'état statique
    if (nouvelEtatMouvement == 1) {
        compteurStatique++;
        if (compteurStatique >= TEMPORISATION_STATIQUE) {
            dernierEtatMouvement = 1; // Confirmer statique
        }
    } else {
        compteurStatique = 0; // Réinitialiser le compteur si en mouvement
        dernierEtatMouvement = 2;
    }

    descriptionMouvement = (dernierEtatMouvement == 2) ? "en mouvement" : "statique";

    // Affichage formaté pour l'accéléromètre
    //printAccelerometerData(ax, ay, az, dernierEtatMouvement, descriptionMouvement);
 }

// === Initialisation ===
void setup() {
    delay(1000);
    printf("\n---------- STARTUP ----------\n");

    pinMode(BUTTON_B_PIN, INPUT_PULLUP); // Configure le bouton B avec un pull-up interne

    // Configure WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("Connecté au WiFi.");

    // Configure TLS
    net.setCACert(root_ca);
    net.setCertificate(certificate);
    net.setPrivateKey(private_key);

    // Configure MQTT 
    client.setServer(aws_endpoint, 8883);
    client.setCallback(callback); // Définir la fonction de rappel

    // Configure sensors
    monServo.attach(SERVO_PIN);
    monServo.write(angleActuel);

    SERIAL.begin(115200);
    while (!SERIAL);

    if (VL53L0X.VL53L0X_common_init() != VL53L0X_ERROR_NONE) {
        SERIAL.println("Failed to initialize VL53L0X!");
        while (1);
    }
    VL53L0X.VL53L0X_continuous_ranging_init();

    WIRE.begin();
    LIS.begin(WIRE, LIS3DHTR_ADDRESS_UPDATED);
    if (!LIS.isConnection()) {
        SERIAL.println("Failed to initialize LIS3DHTR!");
        while (1);
    }
    LIS.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);

    switchHandler.begin();
    AHT.begin();
}

// Fonction pour gérer le mode ON/OFF
void gestionModeONOFF() {
    if (rucheON) {
        // Ruche activée
        Serial.println("La ruche est ALLUMÉE.");
        // Placez ici le code à exécuter lorsque la ruche est en mode ON
        monServo.write(90); // Exemple : régler l'angle du servo sur 90° (au besoin)
    } else {
        // Ruche désactivée
        Serial.println("La ruche est ÉTEINTE.");
        // Placez ici le code à exécuter lorsque la ruche est en mode OFF
        monServo.write(0); // Exemple : fermer complètement le servo
    }
}

void gererBoutonB() {
    int reading = digitalRead(BUTTON_B_PIN);

    // Gérer le rebond (debounce)
    if (reading != lastButtonBState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        // Si l'état du bouton a changé
        if (reading != currentButtonBState) {
            currentButtonBState = reading;

            // Si le bouton est pressé (LOW dans le cas d'un bouton avec pull-up)
            if (currentButtonBState == LOW) {
                // Basculer entre les modes manuel et automatique
                controleAutomatique = !controleAutomatique;
                controleManuel = !controleAutomatique;

                // Afficher le nouveau mode
                if (controleAutomatique) {
                    Serial.println("Mode automatique activé.");
                } else {
                    Serial.println("Mode manuel activé.");
                }
            }
        }
    }

    // Mettre à jour l'état précédent
    lastButtonBState = reading;
}

// === Boucle principale ===
void loop() {
    gererBoutonB(); // Gérer les pressions sur le bouton B

    mesurerRucheEnMouvement();
    mesurerTemperatureHumidite();
    mesurerDistance();
    getGps();

    // Gestion des modes
    if (rucheON) {
        controleManuel = true; // Activer le contrôle manuel si la ruche est ON
    } else {
        gererModeOFF(); // Gérer le mode OFF
    }

    // Contrôle manuel et automatique si activés
    if (controleManuel) {
        controleParSwitch();
    }
    if (controleAutomatique) {
        controleParTemperature();
    }

    // Gérer ouverture et fermeture automatiques
    if (enOuverture) {
        gererOuverture();
    }
    if (enFermeture) {
        gererFermeture();
    }

    // Gestion des données AWS
    if (!client.connected()) {
        connectAWS();
    }
    client.loop();

    // Publication des données périodiques
    unsigned long currentMillis = millis(); // Temps écoulé depuis le démarrage
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis; // Réinitialiser le temps de référence
        sendData(dataToSend());
    }

    // Autres tâches
    delay(10); // Petit délai pour éviter d'occuper le processeur
    printAllData();
}
