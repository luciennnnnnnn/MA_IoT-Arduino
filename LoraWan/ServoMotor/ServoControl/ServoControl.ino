#define USE_TINYUSB 0
#define SERIAL_PORT_MONITOR Serial1
#define Serial Serial1  // Ne pas redéfinir Serial si ce n'est pas nécessaire

#include <Arduino.h>
#include <nrf_pwm.h>

// Configuration de la broche PWM
const uint8_t servoPin = 15; // Remplacez par la broche utilisée
const uint16_t pwmFrequency = 50; // Fréquence en Hz (50 Hz pour les servos)

// Plage d'impulsions (en microsecondes)
const uint16_t minPulseWidth = 500;  // 0° -> 500 µs
const uint16_t maxPulseWidth = 2500; // 180° -> 2500 µs

NRF_PWM_Type* pwmInstance = NRF_PWM0; // Utiliser le périphérique PWM0
uint16_t pwmSeqBuffer[1];             // Buffer pour stocker les séquences PWM

void setup() {
  while (!Serial1); // Attendre que Serial1 soit prêt
  Serial1.begin(9600); // Initialisation du port série Serial1

  // Configurer la broche comme sortie
  pinMode(servoPin, OUTPUT);

  // Configurer le PWM
  pwmInstance->ENABLE = (PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);
  pwmInstance->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_16; // Diviseur pour obtenir 50 Hz

  // Configurer la période (20 ms)
  pwmInstance->COUNTERTOP = 32000; // 20 ms à 16 MHz divisé par 16
  pwmInstance->MODE = PWM_MODE_UPDOWN_Up;
  pwmInstance->LOOP = 0; // Pas de répétition en boucle

  // Configurer le buffer de séquence
  pwmInstance->DECODER = PWM_DECODER_LOAD_Individual | PWM_DECODER_MODE_RefreshCount;
  pwmInstance->SEQ[0].PTR = (uint32_t)&pwmSeqBuffer[0];
  pwmInstance->SEQ[0].CNT = 1; // Une seule valeur par séquence
  pwmInstance->SEQ[0].REFRESH = 0;
  pwmInstance->SEQ[0].ENDDELAY = 0;

  // Associer la broche au périphérique PWM
  pwmInstance->PSEL.OUT[0] = servoPin;

  // Lancer le PWM
  pwmInstance->TASKS_SEQSTART[0] = 1;
}

void loop() {
  // Balayer de 0° à 180° et revenir
  for (int angle = 0; angle <= 180; angle++) {
    uint16_t pulseWidth = map(angle, 0, 180, minPulseWidth, maxPulseWidth);
    pwmSeqBuffer[0] = pulseWidth * 16; // Convertir µs en ticks (1 tick = 1/16 MHz)
    pwmInstance->TASKS_SEQSTART[0] = 1; // Mettre à jour le PWM
    delay(20); // Attendre un peu pour lisser le mouvement
  }
  for (int angle = 180; angle >= 0; angle--) {
    uint16_t pulseWidth = map(angle, 0, 180, minPulseWidth, maxPulseWidth);
    pwmSeqBuffer[0] = pulseWidth * 16; // Convertir µs en ticks
    pwmInstance->TASKS_SEQSTART[0] = 1; // Mettre à jour le PWM
    delay(20); // Attendre un peu pour lisser le mouvement
  }
}
