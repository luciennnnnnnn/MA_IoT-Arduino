#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "src/aws_config.h"

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnecté au WiFi.");

  net.setCACert(root_ca);

  Serial.println("Test de connexion TLS...");
  if (!net.connect(aws_endpoint, 8883)) {
    Serial.println("Échec de la connexion TLS.");
  } else {
    Serial.println("Connexion TLS réussie !");
  }
}

void loop() {
  // Empty loop
}
