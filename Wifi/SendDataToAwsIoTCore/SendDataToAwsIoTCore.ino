#include "aws_config.h"

void setup() {
  Serial.begin(115200);

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
}

void loop() {
  if (!client.connected()) {
    connectAWS();
  }
  client.loop();

  // Publier des données au format JSON
  StaticJsonDocument<128> doc;
  doc["temperature"] = random(20, 30);
  doc["humidity"] = random(50, 70);

  char payload[256];
  serializeJson(doc, payload);
  client.publish("test/topic", payload);
  Serial.println("Données publiées : ");
  Serial.println(payload);

  delay(5000);
}