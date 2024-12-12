#include <CayenneLPP.h>
#include "Seeed_vl53l0x.h"
#include "LIS3DHTR.h"
//#include <Wire.h>
#include "src/SwitchHandler.h"
#include "AHT20.h"
#include <math.h> // Pour sqrt et fabs
#include <Arduino.h>

#include <LbmWm1110.hpp>
#include <Lbmx.hpp>

#undef SERIAL // Facultatif : supprimer la définition précédente
#define MY_SERIAL Serial

#define SEUIL_FERME 100    // Distance en mm pour considérer "fermé"
#define SEUIL_OUVERT 500   // Distance en mm pour considérer "ouvert"
#define SEUIL_NORME 0.3    // Seuil pour la variation de la norme vectorielle
#define TEMPORISATION_STATIQUE 5 // Nombre de cycles requis pour confirmer un état statique

float lastNorme = 0.0; // Dernière valeur de la norme vectorielle
int compteurStatique = 0; // Compteur pour temporisation
int dernierEtatMouvement = 1; // Dernier état connu (1 = statique, 2 = en mouvement)

// Fonction pour calculer la norme vectorielle
float calculerNorme(float x, float y, float z) {
    return sqrt(x * x + y * y + z * z);
}

// Fonction pour calculer une moyenne glissante
float calculerMoyenne(float ancienneValeur, float nouvelleValeur, float facteur) {
    return (ancienneValeur * (1.0 - facteur)) + (nouvelleValeur * facteur);
}

// VL53L0X Time-of-Flight sensor object
Seeed_vl53l0x VL53L0X;

// Define the serial port depending on the board
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

// Define LIS3DHTR accelerometer object
LIS3DHTR<TwoWire> LIS;
#define WIRE Wire

// Define pin for the switch and switch handler object
#define SWITCH_PIN 1
SwitchHandler switchHandler(SWITCH_PIN);

// AHT20 temperature and humidity sensor object
AHT20 AHT;

// CayenneLPP object for formatting data
CayenneLPP lpp(51);

////////////////////////////////////////////////////////////////////////////////
// Types

enum class StateType
{
    Startup,
    Joining,
    Joined,
    Failed,
};

////////////////////////////////////////////////////////////////////////////////
// Constants

static constexpr smtc_modem_region_t REGION = SMTC_MODEM_REGION_EU_868;
static const uint8_t DEV_EUI[8]  = { 0x2C, 0xF7, 0xF1, 0xF0, 0x61, 0x90, 0x00, 0x67 };
static const uint8_t JOIN_EUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t APP_KEY[16] = { 0x54, 0x98, 0x8D, 0x11, 0xBD, 0xC0, 0xDD, 0xD1, 0xAB, 0x2F, 0x69, 0x91, 0x71, 0xFD, 0x40, 0x2A };

static constexpr uint32_t FIRST_UPLINK_DELAY = 10;  // [sec.]
static constexpr uint32_t UPLINK_PERIOD = 30;       // [sec.]
static constexpr uint8_t UPLINK_FPORT = 3;

static constexpr uint32_t EXECUTION_PERIOD = 50;    // [msec.]

static constexpr uint8_t SEND_BYTES_LENGTH = 51;
////////////////////////////////////////////////////////////////////////////////
// Variables

static LbmWm1110& lbmWm1110 = LbmWm1110::getInstance();
static StateType state = StateType::Startup;
////////////////////////////////////////////////////////////////////////////////
// MyLbmxEventHandlers

class MyLbmxEventHandlers : public LbmxEventHandlers
{
protected:
    void reset(const LbmxEvent& event) override;
    void joined(const LbmxEvent& event) override;
    void joinFail(const LbmxEvent& event) override;
    void alarm(const LbmxEvent& event) override;

};

void MyLbmxEventHandlers::reset(const LbmxEvent& event)
{
    if (LbmxEngine::setRegion(REGION) != SMTC_MODEM_RC_OK) abort();
    if (LbmxEngine::setOTAA(DEV_EUI, JOIN_EUI, APP_KEY) != SMTC_MODEM_RC_OK) abort();

    printf("Join the LoRaWAN network.\n");
    if (LbmxEngine::joinNetwork() != SMTC_MODEM_RC_OK) abort();

    // if((REGION == SMTC_MODEM_REGION_EU_868) || (REGION == SMTC_MODEM_REGION_RU_864))
    // {
    //     smtc_modem_set_region_duty_cycle( false );
    // }

    state = StateType::Joining;
}

void MyLbmxEventHandlers::joined(const LbmxEvent& event)
{
    state = StateType::Joined;
    printf("Start the alarm event.\n");
    if (LbmxEngine::startAlarm(FIRST_UPLINK_DELAY) != SMTC_MODEM_RC_OK) abort();
}

void MyLbmxEventHandlers::joinFail(const LbmxEvent& event)
{
    state = StateType::Failed;
}

void MyLbmxEventHandlers::alarm(const LbmxEvent& event)
{
    printf("Send the uplink message.\n");

    CayenneLPP lpp(SEND_BYTES_LENGTH);
    lpp.reset();
    lpp.addTemperature(1, random(20,30));
    lpp.addRelativeHumidity(2, random(40, 80));
    lpp.addPresence(3, random(0, 1));

    if (LbmxEngine::requestUplink(UPLINK_FPORT, false, lpp.getBuffer(), lpp.getSize()) != SMTC_MODEM_RC_OK) abort();

    if (LbmxEngine::startAlarm(UPLINK_PERIOD) != SMTC_MODEM_RC_OK) abort();
}

////////////////////////////////////////////////////////////////////////////////
// ModemEventHandler

static void ModemEventHandler()
{
    static LbmxEvent event;
    static MyLbmxEventHandlers handlers;

    while (event.fetch())
    {
        printf("----- %s -----\n", event.getEventString().c_str());

        handlers.invoke(event);
    }
}
void setup() {

    delay(1000);
    printf("\n---------- STARTUP ----------\n");
    
    lbmWm1110.begin();
    LbmxEngine::begin(lbmWm1110.getRadio(), ModemEventHandler);
    LbmxEngine::printVersions(lbmWm1110.getRadio());

    // Initialize serial communication for debugging
    SERIAL.begin(115200);
    while (!SERIAL) {
        ; // Wait for the serial connection to establish
    }

    // Initialize VL53L0X
    VL53L0X_Error Status = VL53L0X.VL53L0X_common_init();
    if (Status != VL53L0X_ERROR_NONE) {
        SERIAL.println("Failed to initialize VL53L0X!");
        VL53L0X.print_pal_error(Status);
        while (1);
    }

    VL53L0X.VL53L0X_continuous_ranging_init();
    if (Status != VL53L0X_ERROR_NONE) {
        SERIAL.println("Failed to start continuous ranging for VL53L0X!");
        VL53L0X.print_pal_error(Status);
        while (1);
    }

    // Initialize I2C bus
    WIRE.begin();

    // Initialize LIS3DHTR accelerometer
    LIS.begin(WIRE, LIS3DHTR_ADDRESS_UPDATED); // The method returns void
    if (!LIS.isConnection()) { // Use a method that verifies the connection
        SERIAL.println("Failed to initialize LIS3DHTR!");
        while (1);
    }
    LIS.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);

    // Initialize the switch handler
    switchHandler.begin();

    // Initialize AHT20
    SERIAL.println("Initializing AHT20 sensor...");
    AHT.begin(); // Simple initialization, no return value

    delay(100);  // Allow the sensor to stabilize after initialization

    // Perform a quick test to ensure the sensor is operational
    float humidity, temperature;
    int status = AHT.getSensor(&humidity, &temperature);
    if (status == 0) { // 0 indicates failure
        SERIAL.println("Failed to initialize AHT20!");
        while (1); // Stop execution if the sensor initialization fails
    }

    SERIAL.println("All sensors initialized successfully!");
}

void loop() {
    // Lire l'état du switch
    bool switchState = switchHandler.readSwitch();
    const char* descriptionSwitch = switchState ? "pressé" : "relaché";

    // Afficher l'état du switch
    SERIAL.print("État du switch : ");
    SERIAL.println(descriptionSwitch);

    // ----- Capteur d'accélération -----
    if (!LIS.isConnection()) {
        SERIAL.println("LIS3DHTR disconnected!");
        while (1);
    }

    // Lire les valeurs de l'accéléromètre
    float ax = LIS.getAccelerationX();
    float ay = LIS.getAccelerationY();
    float az = LIS.getAccelerationZ();

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

    const char* descriptionMouvement = (dernierEtatMouvement == 2) ? "en mouvement" : "statique";

    // Affichage formaté pour l'accéléromètre
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

    // ----- Capteur de distance -----
    VL53L0X_RangingMeasurementData_t RangingMeasurementData;
    VL53L0X.PerformContinuousRangingMeasurement(&RangingMeasurementData);

    int etatDistance = 0; // Par défaut
    const char* descriptionDistance = "";
    if (RangingMeasurementData.RangeMilliMeter >= 2000) {
        SERIAL.println("Distance: Hors de portée");
        etatDistance = 3; // Considérer comme "ouvert" si hors de portée
        descriptionDistance = "ouvert";
    } else {
        int distance = RangingMeasurementData.RangeMilliMeter;
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
        SERIAL.print("Distance du capteur : ");
        SERIAL.print(distance);
        SERIAL.print(" mm, État de la porte : ");
        SERIAL.print(etatDistance);
        SERIAL.print(" (");
        SERIAL.print(descriptionDistance);
        SERIAL.println(")");
    }

    // ----- Capteur de température et d'humidité -----
    float humidity = 0, temperature = 0;
    bool ahtSuccess = AHT.getSensor(&humidity, &temperature);
    if (ahtSuccess) {
        SERIAL.print("Température : ");
        SERIAL.print(temperature);
        SERIAL.print(" °C, Humidité : ");
        SERIAL.print(humidity * 100); // Multiplier par 100 pour obtenir le pourcentage
        SERIAL.println(" %");
    } else {
        SERIAL.println("Erreur de lecture du capteur AHT20 !");
    }

    // ----- Ajout des données au payload CayenneLPP -----
    lpp.addDigitalInput(6, dernierEtatMouvement); // État de mouvement
    lpp.addDigitalInput(7, etatDistance);         // État de la distance
    if (ahtSuccess) {
        lpp.addTemperature(8, temperature);
        lpp.addRelativeHumidity(9, humidity * 100); // Multiplier par 100 pour un format correct
    }

    // Transmettre le payload via le moniteur série
    SERIAL.print("Payload CayenneLPP : ");
    for (size_t i = 0; i < lpp.getSize(); i++) {
        SERIAL.print(lpp.getBuffer()[i], HEX);
        SERIAL.print(" ");
    }
    SERIAL.println();

    // Réinitialiser le buffer CayenneLPP
    lpp.reset();

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

    delay(30000);
}