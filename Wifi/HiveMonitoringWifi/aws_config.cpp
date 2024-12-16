#include "aws_config.h"

WiFiClientSecure net; // Objet pour gérer la connexion sécurisée
PubSubClient client(net); // Initialisation du client MQTT
const char* ThingName = "ESP32_Device1";
const char* subscribeTopic = "subscribe/esp32/downlink";
const char* publishTopic = "publish/esp32/uplink";

// Configuration WiFi
const char* ssid = "";
const char* password = "";

// Endpoint AWS IoT Core
const char* aws_endpoint = "a2v2z8f3sbcm46-ats.iot.eu-west-1.amazonaws.com";

// Certificats et clé privée
const char* certificate = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVAPP7vgs8GPa3D4C0bAsJytt3LO9tMA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yNDEyMTYxNDI2
MDVaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDEgHzX0KyeTZ6DS1Ia
RFpXtgCIOOEiQRkWm7UiXlJa+a5Dwdrc2eSc1MXQRUAM3/0YfMEtofr+VNUOFXXy
im7yA+fowUCPhS7XEwx5acCfbwvcuKp9ziRAIk2/BsFDoPK5gGhK2ME/nERLQYbY
/8i3rghrtCyJJw8G/SAVoBq6PRv2Tg6rKfvIW19vR9Yev5L2AP1Z6cauP7Ncccfp
jxL+Rk4L4AQC/p4ims6bh8YzEuVPnsYxRmtKhJkQ4IHqjdhpwGlVpy/ylcwHjfsr
dVjMjWrFV8KYEah9MBHK22hEeBBVaMGmdSyc/O/X/+ykt+1eKpjAFvek7gvZ7jUV
yicFAgMBAAGjYDBeMB8GA1UdIwQYMBaAFK/mKozzzlv9R9hHftk243D9CxfXMB0G
A1UdDgQWBBSKD48IzNzanz5479STYydV+vfHaDAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAPMtCJP/PJn74la7nzHLEaOQQ
FCwiorAh/aR9sxJbZLQZvR1AW7DopZJkkm8Q2uWpS0kDEjqjq2RFy2G4ExGpG7kv
BkwEdzTofa++fQ4PUBB1yLDoF1X9D+NVh2GIqBU3LJ41VgGaZhupvSoNSpT2eiJA
Vx0Gix6I6DDTJkMnPe9vf2Zws+6KxgZOJa9WwpDFgm77qVmhVq5nrxeK7XdG0Wqx
4CsptDDEvHemKf9uZsN/E5LjMnvi2VpKCGTqvBdOka7wDz3VwAILABgZSqZDbG7F
WNglMsHKeXlP+N8WISIQ+QapgmTpO1WKG4tWJywObdDiVE7fQzZWbufxC3VHaQ==
-----END CERTIFICATE-----)EOF";

const char* private_key = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEogIBAAKCAQEAxIB819Csnk2eg0tSGkRaV7YAiDjhIkEZFpu1Il5SWvmuQ8Ha
3NnknNTF0EVADN/9GHzBLaH6/lTVDhV18opu8gPn6MFAj4Uu1xMMeWnAn28L3Liq
fc4kQCJNvwbBQ6DyuYBoStjBP5xES0GG2P/It64Ia7QsiScPBv0gFaAauj0b9k4O
qyn7yFtfb0fWHr+S9gD9WenGrj+zXHHH6Y8S/kZOC+AEAv6eIprOm4fGMxLlT57G
MUZrSoSZEOCB6o3YacBpVacv8pXMB437K3VYzI1qxVfCmBGofTARyttoRHgQVWjB
pnUsnPzv1//spLftXiqYwBb3pO4L2e41FconBQIDAQABAoIBAEGWrZUPpv5z5Avo
8S+pTwplyomtip/3YJtmP7CIES8ileMx6o67AA7FCn35D2b8wKsSwRwtK+0GfdLk
oOjjAiMv4JGVJ6qABuFtD0XbYZsV4ry6utWhIAJ0MMUpSQr8xufaD3WkIeTMkRMZ
fRwjTGja4x6oFhF1gxOPG818Ek/FDeUlXlBcCbob30bthaHiuw2/wmOWe1gGomQC
wlXmCP+CIRc9XCU35GKSJb/6N+6Z6v3dfmNo/+b+q0SyqKQwNYcr1Ze8YJ2zj9Eg
w5KShi9TCmvlxhsV48RQm4VAzUd0JCEVkhxDZPg4NNxbndGwVuFYAEV+SL4gq+UC
VCAwWQUCgYEA7//8cplWr2hTtCBkEmlNW/ubhf3BVNOzYw0EcQmtUxvbzgPPWONe
HHLuR0EM1y1vSt8ovuHsWO8WYcbbdTETAmVT3YO+eLDUmTt9kYbj9FQ6hIhrV1Fj
vwQzJ1J8F9QE+c25pxx43bhraq1pfuEGPwx+gfFbkRcs/xaqLmfXBHMCgYEA0Zoh
3kme2Fns/myYClqpr9/tOr8wZ0qyY9K1P9XrGfF54eLwOT97Uc3uaXx9NIEGKGXC
SwkYVUYu44QLGJrO8qBWQUcdDwEDLE5AaNrVPJyimUrb/PpybZhw26fwz0KXxQdX
n15N+sz2Mux8lje23s7D9KoedmgB7iPexXoQwKcCgYB9iBCcRHYh41i8w6B0PN34
ub8cM6E25FBVA88J7Pzalpy+wiFnJe3oxXi6jlzQA/TNXbIrPXJpBg1X8TKCtrtx
qlxWZf1vlVZuG77PKpX7Cz9caRp0Hp8kn2a+L5e2wvAjVSbBMNRLbJXKw4+FjN++
hKN7BipV1qBzgIBdK3U83wKBgAgAgGaCHQRZeojEEJHkoDHMtLvXbLukyGazNqQ3
QIB/DY4oIc+tssGOwRlZ3VXRzzBfoilHMZyfz050ThAkaN4ubNyVIHDgMwTP//mA
0T89I6ACkZ942EGC3lwkbzcabbzh6OB2szdt98A98wuxGNQsxKJklEEk+13iFLEp
VdZlAoGAfbKY5zqkXSO95G6sSPVHse9s16Lqo5OK9bDGtJjpo75bvg3IT79/itC+
qt+Hb8Z0K4/tvL4N0xv0V3JEIuwlhBgxC/KWDv9Itjv6AtsdrMTMHTa1my7rcqs/
b6t5r5H4L8S56dWXc3VNuaKURNMucl3/Bfo4m3V/UqOzJ8YGdA8=
-----END RSA PRIVATE KEY-----)EOF";
 


const char* root_ca = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----)EOF";

// Fonction de rappel pour les messages MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message reçu sur le topic : ");
  Serial.println(topic);

  // Convertir le payload en chaîne de caractères
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  Serial.print("Message : ");
  Serial.println(message);

  // Si nécessaire, traiter le message JSON
  StaticJsonDocument<128> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (!error) {
    // Exemple : Afficher une valeur si le message est JSON
    if (doc.containsKey("command")) {
      Serial.print("Commande reçue : ");
      Serial.println(doc["command"].as<String>());
    }
  } else {
    Serial.println("Erreur de parsing du message JSON.");
  }
}

// Fonction pour se connecter à AWS IoT Core
void connectAWS() {
  while (!client.connected()) {
    Serial.print("Connexion à AWS IoT Core...");
    if (client.connect(ThingName)) {
      Serial.println("Connecté !");
      // S'abonner au sujet une fois connecté
      client.subscribe(subscribeTopic);
      Serial.println("Abonné au sujet : ");
      Serial.println(subscribeTopic);
    } else {
      Serial.print("Echec, erreur: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}