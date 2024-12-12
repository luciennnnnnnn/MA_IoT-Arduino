// Constantes
const int switchPin = 1; // Pin où le switch est connecté

// Variables
int switchState = 0;

void setup() {
  // Configurer la pin du switch en entrée
  pinMode(switchPin, INPUT);
  
  // Démarrage du moniteur série
  Serial.begin(9600);
}

void loop() {
  // Lire l'état du switch (HIGH ou LOW)
  switchState = digitalRead(switchPin);
  
  // Afficher un message en fonction de l'état du switch
  if (switchState == HIGH) {
    Serial.println("On"); // Switch enclenché
  } else {
    Serial.println("Off"); // Switch relâché
  }

  // Petite pause pour lisibilité
  delay(100);
}
