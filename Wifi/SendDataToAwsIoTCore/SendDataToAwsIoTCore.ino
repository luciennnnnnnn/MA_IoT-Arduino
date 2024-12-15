#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Configuration WiFi
const char* ssid = "vmd-11209";
const char* password = "49vw-87kg-nrcb-zsxv";

// Endpoint AWS IoT Core
const char* aws_endpoint = "a2v2z8f3sbcm46-ats.iot.eu-west-1.amazonaws.com";

WiFiClientSecure net; // Objet pour gérer la connexion sécurisée

// Certificats et clé privée
const char* certificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDWjCCAkKgAwIBAgIVAMix8rqDNhFoUZZHo6SVo0VTQ1uIMA0GCSqGSIb3DQEB\n" \
"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n" \
"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yNDEyMTUxMzE4\n" \
"MDlaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n" \
"dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC7+Om4id+DXPD1chrh\n" \
"5tGQBfhd8RqUe7dz5FX/FYaCptiX7uubvPbGhZqSfipZ0uaxUxQuDb1BivaXoGTH\n" \
"KDMDYMm0kNY3SMpLWxJiXGqV0ISCWYwZErOZEfegk61RZLt28u+kl1ZfPlE993Cd\n" \
"8rGTeZmWFTXInPSkaMUN2uPFh95vOHzs/8L+Ehh42lBFzWYm0NLHhAAvNV6S6lW1\n" \
"qx0a5D+58jpEO/6RnWu2KApEuuEE2ru2kIq1WCDSYesAE7N6Yq1Y982dmaJPIfkZ\n" \
"Ym7SeZXdFORASbtILtBKEjftfA4THmmAzgus/SOIa7RrazsKJdiJVJdDAagQ1xRn\n" \
"uxsDAgMBAAGjYDBeMB8GA1UdIwQYMBaAFBbzZhTm1/ovPxoUHVbrtTYA6JIxMB0G\n" \
"A1UdDgQWBBRr1xTYpvJaHzKMN4+VoZy0LecOQTAMBgNVHRMBAf8EAjAAMA4GA1Ud\n" \
"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEALWrgXV/HnFXb/89FqsUushIV\n" \
"corVvmauZ5tzNtmXabAZXKmpLqkWgr4Un9I/zHE/0jj9QHaGTHP002aNDnG94lRw\n" \
"4WPmQ06FmhkLR1Hvryg9K/XryBQ3WUVwVyAeOoTl1vaPxQP6zwHzrK1IazDr/akD\n" \
"MQ3wbVaqUJy29rsOS4hZu1o8ZTnpPBoJXG2mRaK/v9Z+cd0IR8daUPbJUBdVkTdi\n" \
"6Eiwhc76RvXMwiOc80V8ls6du6XsHegZCQF8cFeh656GCfxZEVbvza0EXlhsClLy\n" \
"bbH+BkGivv3EeWGvTMSX7meqvWpbHLJ+mwXWwaEkfkAhE2+7YCkiWpUE1xvXEQ==\n" \
"-----END CERTIFICATE-----\n";

const char* private_key = \
"-----BEGIN PRIVATE KEY-----\n" \
"MIIEpAIBAAKCAQEAu/jpuInfg1zw9XIa4ebRkAX4XfEalHu3c+RV/xWGgqbYl+7r\n" \
"m7z2xoWakn4qWdLmsVMULg29QYr2l6BkxygzA2DJtJDWN0jKS1sSYlxqldCEglmM\n" \
"GRKzmRH3oJOtUWS7dvLvpJdWXz5RPfdwnfKxk3mZlhU1yJz0pGjFDdrjxYfebzh8\n" \
"7P/C/hIYeNpQRc1mJtDSx4QALzVekupVtasdGuQ/ufI6RDv+kZ1rtigKRLrhBNq7\n" \
"tpCKtVgg0mHrABOzemKtWPfNnZmiTyH5GWJu0nmV3RTkQEm7SC7QShI37XwOEx5p\n" \
"gM4LrP0jiGu0a2s7CiXYiVSXQwGoENcUZ7sbAwIDAQABAoIBAHNtYnhDkdFk3UKb\n" \
"kw5tYvqB8AYf7CaJ4hl0eapuWW8ZbZ++w0ebPh6iJE0Y1IDcFXWGgrnVDgaVgt+R\n" \
"StFRSiIk6TfMSP8h37ducVedUIY2yWgR/QnYhhjVA67DgIaeyM8+VxJHLG7IIbNf\n" \
"ovQa+BiLb1jOJXzPhRsAiyD67St6OfD5y4zgClIymU5m632akQgZmpTb/K6Adb3A\n" \
"4tz83DFN//xnQ55ehuHuFt6XyIyQ5ZapWzpZ72TZLxnMTtSivJnIy4bkZqmZF+YK\n" \
"5lA/eb4JWbrxOeKF5eKAqxr3J+9fAH6ppouiO3WjfXOxYJReWcxQs9KzQRLUBeoo\n" \
"A2CGRuECgYEA45Jdhc4PPUz+YVwVSlpLpCiUV5SsLBH8WoJY5hKTUHMNvFxF2n1W\n" \
"HHQrKWm3UQcXT3tZVlLSGdIyisOlJbp7ZkjWHkERZOXV/U9RVk2ZSWUo3Uod/DOL\n" \
"HkwTw+sXHsTqvh64OMVvB5y3BtB28LeJEbMonar+WFdyd78vVkLnMYUCgYEA03Qt\n" \
"WikRtRTDirr1Wy6d/BeP/9ogT5t49QNE63CCr3tsP1y2EYYaFodD17S/QG08p/Tw\n" \
"p3I277Y7rCzxsRaQIDqzRw+oCLOx0ts1I/wRjKWa3luT68QtbDRfj5eUxC6n23U2\n" \
"ZbmpUgb3v3572V7akK34EFQgbnaVKd4ZI/HtfOcCgYAGmBgjp2vjn9aDJbLsO5+T\n" \
"eyWJQCKZCFBiowJGcGVaYGsWeSoBAb68BlW11n1cWLNBN1bIXI//9YpHYRHo/PVU\n" \
"amnh6IrDfLFtrJ2zvX3ilY7QkrQ5uZifYR71f1tlJdmTdrpp0ra75rua8/FuWjMb\n" \
"kzBrGShTkP0dISoS+s9OLQKBgQC3xUdJDVpbUxFit3tXZJtfJSc8B9l+9jLYVaAH\n" \
"MoARfKWmJ7AAaNixbZtzk52Ho6hMnB6umB/Qk0MnLuRasK5PpFR81MmHzowkIej8\n" \
"Pi0OXDrNNy/Gfnevi5DlnapoGdg+bY76N7JFHNMiNQ3z63DuvpNRtZt+BUc73M9i\n" \
"ovZwTwKBgQDKexmL025S9uu8VGLENyWQz8HSATOJpmjtC/l4RynnyXv46pu/fZTJ\n" \
"HjMxE67jUgCTE5iLrrITSisIydrXWJMAqLjwe7qUgM3V0yhGNY6nHv5kY4rAKp8X\n" \
"9i2IdP2LvanJe1sUXHKhIH+/Fze2vx7bwEekrsvtnhY5FmBaspIlBA==\n" \
"-----END PRIVATE KEY-----\n";

const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
"rqXRfboQnoZsG4q5WTP468SQvvG5==\n" \
"-----END CERTIFICATE-----\n";

PubSubClient client(net); // Initialisation du client MQTT

void connectAWS() {
  while (!client.connected()) {
    Serial.print("Connexion à AWS IoT Core...");
    if (client.connect("ESP32_Device")) {
      Serial.println("Connecté !");
    } else {
      Serial.print("Echec, erreur: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

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
}



void loop() {
  if (!client.connected()) {
    connectAWS();
  }
  client.loop();

  // Publish data
  StaticJsonDocument<128> doc;
  doc["temperature"] = random(20, 30);
  doc["humidity"] = random(50, 70);

  char payload[256];
  serializeJson(doc, payload);
  client.publish("esp32/topic", payload);

  delay(5000);
}
