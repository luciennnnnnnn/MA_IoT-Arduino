#include <CayenneLPP.h>
#include "Seeed_vl53l0x.h"
#include "LIS3DHTR.h"
#include "src/SwitchHandler.h"
#include "AHT20.h"
#include <math.h> // For sqrt and fabs
#include <Arduino.h>
#include <LbmWm1110.hpp>
#include <Lbmx.hpp>

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
#define SWITCH_PIN 14
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

// LoRaWAN constants and variables
enum class StateType { Startup, Joining, Joined, Failed };
static constexpr smtc_modem_region_t REGION = SMTC_MODEM_REGION_EU_868;
static const uint8_t DEV_EUI[8]  = { 0x2C, 0xF7, 0xF1, 0xF0, 0x61, 0x90, 0x00, 0x67 };
static const uint8_t JOIN_EUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t APP_KEY[16] = { 0x54, 0x98, 0x8D, 0x11, 0xBD, 0xC0, 0xDD, 0xD1, 0xAB, 0x2F, 0x69, 0x91, 0x71, 0xFD, 0x40, 0x2A };

static constexpr uint32_t FIRST_UPLINK_DELAY = 10;  // [sec.]
//static constexpr uint32_t UPLINK_PERIOD = 30;      // [sec.] (debug)
static constexpr uint32_t UPLINK_PERIOD = 600;      // [sec.] avoid network saturation
static constexpr uint8_t UPLINK_FPORT = 3;
static constexpr uint32_t EXECUTION_PERIOD = 50;   // [msec.]
static constexpr uint8_t SEND_BYTES_LENGTH = 51;

static LbmWm1110& lbmWm1110 = LbmWm1110::getInstance();
static StateType state = StateType::Startup;

class MyLbmxEventHandlers : public LbmxEventHandlers {
protected:
    void reset(const LbmxEvent& event) override;
    void joined(const LbmxEvent& event) override;
    void joinFail(const LbmxEvent& event) override;
    void alarm(const LbmxEvent& event) override;
};

void MyLbmxEventHandlers::reset(const LbmxEvent& event) {
    if (LbmxEngine::setRegion(REGION) != SMTC_MODEM_RC_OK) abort();
    if (LbmxEngine::setOTAA(DEV_EUI, JOIN_EUI, APP_KEY) != SMTC_MODEM_RC_OK) abort();

    printf("Joining the LoRaWAN network...\n");
    if (LbmxEngine::joinNetwork() != SMTC_MODEM_RC_OK) abort();
    state = StateType::Joining;
}

void MyLbmxEventHandlers::joined(const LbmxEvent& event) {
    state = StateType::Joined;
    printf("Network joined successfully. Starting alarm event.\n");
    if (LbmxEngine::startAlarm(FIRST_UPLINK_DELAY) != SMTC_MODEM_RC_OK) abort();
}

void MyLbmxEventHandlers::joinFail(const LbmxEvent& event) {
    state = StateType::Failed;
}

void MyLbmxEventHandlers::alarm(const LbmxEvent& event) {
    printf("Sending uplink message.\n");
    CayenneLPP lpp(SEND_BYTES_LENGTH);
    lpp = dataToSend(lpp);

    if (LbmxEngine::requestUplink(UPLINK_FPORT, false, lpp.getBuffer(), lpp.getSize()) != SMTC_MODEM_RC_OK) abort();
    if (LbmxEngine::startAlarm(UPLINK_PERIOD) != SMTC_MODEM_RC_OK) abort();
}

static void ModemEventHandler() {
    static LbmxEvent event;
    static MyLbmxEventHandlers handlers;

    while (event.fetch()) {
        printf("----- %s -----\n", event.getEventString().c_str());
        handlers.invoke(event);
    }
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
    delay(1000);
    printf("\n---------- STARTUP ----------\n");

    lbmWm1110.begin();
    LbmxEngine::begin(lbmWm1110.getRadio(), ModemEventHandler);
    LbmxEngine::printVersions(lbmWm1110.getRadio());

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
    //printSwitchData();

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

     switch (state)
    {
    case StateType::Startup:
        ledOff(LED_BUILTIN);
        break;
    case StateType::Joining:
        if (millis() % 1000 < 200) ledOn(LED_BUILTIN); else ledOff(LED_BUILTIN);
        break;
    case StateType::Joined:
        ledOn(LED_BUILTIN);
        break;
    case StateType::Failed:
        if (millis() % 400 < 200) ledOn(LED_BUILTIN); else ledOff(LED_BUILTIN);
        break;
    }

    const uint32_t sleepTime = LbmxEngine::doWork();

    delay(min(sleepTime, EXECUTION_PERIOD));

    //printAllData();
}