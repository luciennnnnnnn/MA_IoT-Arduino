#include "LIS3DHTR.h"
#include <Wire.h>

// Définir un objet LIS3DHTR pour l'interface I2C
LIS3DHTR<TwoWire> LIS;
#define WIRE Wire

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Attendre que la connexion série soit établie
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

  delay(500); // Pause de 500 ms
}
