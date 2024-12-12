#include <CayenneLPP.h>
#include "Seeed_vl53l0x.h"
#include "LIS3DHTR.h"
#include "src/SwitchHandler.h"
#include "AHT20.h"
#include <math.h> // For sqrt and fabs

// Define serial port
#undef SERIAL
#define SERIAL Serial

// Define constants for thresholds
#define SEUIL_FERME 100         // Distance in mm to consider "closed"
#define SEUIL_OUVERT 500        // Distance in mm to consider "open"
#define SEUIL_NORME 0.3         // Threshold for vector norm variation
#define TEMPORISATION_STATIQUE 5 // Cycles needed to confirm static state

// Global variables
float humidity = 0, temperature = 0;
int etatDistance = 0;
float lastNorme = 0.0;
int compteurStatique = 0;
int dernierEtatMouvement = 1;
float ax = 0;
float ay = 0;
float az = 0;
int distance  = 0;
const char* descriptionDistance = "";
const char* descriptionSwitch = "";
const char* descriptionMouvement = "";

// Function to calculate vector norm
float calculerNorme(float x, float y, float z) {
    return sqrt(x * x + y * y + z * z);
}

// Function to calculate a moving average
float calculerMoyenne(float ancienneValeur, float nouvelleValeur, float facteur) {
    return (ancienneValeur * (1.0 - facteur)) + (nouvelleValeur * facteur);
}

// VL53L0X Time-of-Flight sensor object
Seeed_vl53l0x VL53L0X;

// LIS3DHTR accelerometer object
LIS3DHTR<TwoWire> LIS;
#define WIRE Wire

// Define pin for the switch and switch handler object
#define SWITCH_PIN 1
SwitchHandler switchHandler(SWITCH_PIN);

// AHT20 temperature and humidity sensor object
AHT20 AHT;

// Function to prepare CayenneLPP payload
CayenneLPP dataToSend(CayenneLPP lpp) {
    lpp.reset();

    printAllData();

    lpp.addDigitalInput(1, dernierEtatMouvement);
    lpp.addDigitalInput(2, etatDistance);
    lpp.addTemperature(3, temperature);
    lpp.addRelativeHumidity(4, humidity * 100);

    SERIAL.print("CayenneLPP Payload: ");
    for (size_t i = 0; i < lpp.getSize(); i++) {
        SERIAL.print(lpp.getBuffer()[i], HEX);
        SERIAL.print(" ");
    }
    SERIAL.println();

    return lpp;
}

void printAccelerometerData(float ax, float ay, float az, int dernierEtatMouvement,  const char* descriptionMouvement)
{
     SERIAL.print("Accéléromètre : X");
    SERIAL.print(ax, 2); // Limiter à 2 décimales
    SERIAL.print(" Y");
    SERIAL.print(ay, 2);
    SERIAL.print(" Z");
    SERIAL.print(az, 2);
    SERIAL.print(", État de la ruche : ");
    SERIAL.print(dernierEtatMouvement);
    SERIAL.print(" (");
    SERIAL.print(descriptionMouvement);
    SERIAL.println(")");
}

void printDistanceData(float distance, const char* descriptionDistance)
{
    SERIAL.print("Distance du capteur : ");
    SERIAL.print(distance);
    SERIAL.print(" mm, État de la porte : ");
    SERIAL.print(etatDistance);
    SERIAL.print(" (");
    SERIAL.print(descriptionDistance);
    SERIAL.println(")");
}

void printTempHumidityData()
{
    SERIAL.print("Température : ");
    SERIAL.print(temperature);
    SERIAL.print(" °C, Humidité : ");
    SERIAL.print(humidity * 100); // Multiplier par 100 pour obtenir le pourcentage
    SERIAL.println(" %");
}

void printSwitchData()
{
    SERIAL.print("État du switch : ");
    SERIAL.println(descriptionSwitch);
}

void printAllData()
{
    printAccelerometerData(ax, ay, az, dernierEtatMouvement, descriptionMouvement);
    printDistanceData(distance, descriptionDistance);
    printTempHumidityData();
}

void setup() {
    // Initialize serial communication for debugging
    SERIAL.begin(115200);
    while (!SERIAL);

    // Initialize sensors
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

void loop() {
    // Lire l'état du switch
    bool switchState = switchHandler.readSwitch();
    descriptionSwitch = switchState ? "pressé" : "relaché";

    // Afficher l'état du switch
    printSwitchData();

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

    // ----- Capteur de distance -----
    VL53L0X_RangingMeasurementData_t RangingMeasurementData;
    VL53L0X.PerformContinuousRangingMeasurement(&RangingMeasurementData);

    etatDistance = 0; // Par défaut
    descriptionDistance = "";
    if (RangingMeasurementData.RangeMilliMeter >= 2000) {
        SERIAL.println("Distance: Hors de portée");
        etatDistance = 3; // Considérer comme "ouvert" si hors de portée
        descriptionDistance = "ouvert";
    } else {
        distance = RangingMeasurementData.RangeMilliMeter;
        if (distance <= SEUIL_FERME) {
            etatDistance = 1; // Fermé
            descriptionDistance = "fermé";
        } else if (distance >= SEUIL_OUVERT) {
            etatDistance = 3; // Ouvert
            descriptionDistance = "ouvert";
        } else {
            etatDistance = 2; // En mouvement
            descriptionDistance = "en mouvement";
        }

        // Affichage formaté pour la distance et l'état de la porte
        //printDistanceData(distance, descriptionDistance);
    }

    // ----- Capteur de température et d'humidité -----
    bool ahtSuccess = AHT.getSensor(&humidity, &temperature);
    if (ahtSuccess) {
        //printTempHumidityData();
    } else {
        SERIAL.println("Erreur de lecture du capteur AHT20 !");
    }

    //printAllData();
}