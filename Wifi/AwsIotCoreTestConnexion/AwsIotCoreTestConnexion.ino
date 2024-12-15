#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "src/aws_config.h"

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi.");

  // Set up TLS certificates
  net.setCACert(root_ca);
  net.setCertificate(certificate);
  net.setPrivateKey(private_key);

  // Test connection to AWS IoT Core
  Serial.println("Connecting to AWS IoT Core...");
  if (!net.connect(aws_endpoint, 8883)) {
    Serial.println("TLS connection failed. Possible reasons:");
    Serial.println("1. Invalid Root CA, certificate, or private key.");
    Serial.println("2. Incorrect AWS IoT endpoint.");
    Serial.println("3. Time not synchronized.");
  } else {
    Serial.println("TLS connection successful!");
  }
}

void loop() {
  // Empty loop
}
