#include "Seeed_vl53l0x.h"
Seeed_vl53l0x VL53L0X;

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

#include "LIS3DHTR.h"
#include <Wire.h>

// Définir un objet LIS3DHTR pour l'interface I2C
LIS3DHTR<TwoWire> LIS;
#define WIRE Wire

#include "src/SwitchHandler.h"
// Define the pin for the switch
#define SWITCH_PIN 1

// Create a SwitchHandler object
SwitchHandler switchHandler(SWITCH_PIN);

void setup() {
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    // Initialize serial communication for debugging
    Serial.begin(115200);
    while (!Serial) {
      ; // Attendre que la connexion série soit établie
    }
    Status = VL53L0X.VL53L0X_common_init();
    if (VL53L0X_ERROR_NONE != Status) {
        SERIAL.println("start vl53l0x mesurement failed!");
        VL53L0X.print_pal_error(Status);
        while (1);
    }
    VL53L0X.VL53L0X_continuous_ranging_init();
    if (VL53L0X_ERROR_NONE != Status) {
        SERIAL.println("start vl53l0x mesurement failed!");
        VL53L0X.print_pal_error(Status);
        while (1);
    }

    // Initialiser le bus I2C
    WIRE.begin();  // Utilise les broches par défaut SDA (GPIO 21) et SCL (GPIO 22)
    // Si vous utilisez d'autres broches, spécifiez-les ici, par exemple : WIRE.begin(SDA_PIN, SCL_PIN);

    // Initialiser le capteur avec l'adresse I2C mise à jour
    LIS.begin(WIRE, LIS3DHTR_ADDRESS_UPDATED); // Adresse par défaut : 0x19
    
    // Vérifier si le capteur est connecté
    if (!LIS.isConnection()) {
      Serial.println("Impossible d'initialiser le capteur LIS3DHTR !");
      while (1) {
        ; // Boucle infinie si le capteur n'est pas détecté
      }
    }

    delay(100); // Attendre la stabilisation

    // Configurer la fréquence de sortie des données
    LIS.setOutputDataRate(LIS3DHTR_DATARATE_50HZ); // 50 Hz
    
    // Initialize the switch handler
    switchHandler.begin();
}

void loop() {
  // Vérifier si le capteur est connecté
    if (!LIS.isConnection()) {
      Serial.println("LIS3DHTR non connecté !");
      while (1) {
        ; // Boucle infinie si le capteur se déconnecte
      }
      return;
    }

    // Lire les accélérations sur les 3 axes
    Serial.print("x: ");
    Serial.print(LIS.getAccelerationX());
    Serial.print("  ");
    
    Serial.print("y: ");
    Serial.print(LIS.getAccelerationY());
    Serial.print("  ");
    
    Serial.print("z: ");
    Serial.println(LIS.getAccelerationZ());


    // Read the switch state
    bool isSwitchOn = switchHandler.readSwitch();

    // Print the switch state to the serial monitor
    if (isSwitchOn) {
        Serial.println("Switch is ON");
    } else {
        Serial.println("Switch is OFF");
    }

    VL53L0X_RangingMeasurementData_t RangingMeasurementData;
    VL53L0X.PerformContinuousRangingMeasurement(&RangingMeasurementData);
    if (RangingMeasurementData.RangeMilliMeter >= 2000) {
        SERIAL.println("out of ranger");
    } else {
        SERIAL.print("distance::");
        SERIAL.println(RangingMeasurementData.RangeMilliMeter);
    }
    // Add a small delay to avoid flooding the serial monitor
    delay(500);
}
