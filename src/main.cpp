#include <Arduino.h>
#include <ezButton.h>
#include <RF24.h>
#include <JoystickData.h>
#include <RemoteData.h>
#include "Button.h"
#include "Joystick.h"
#include "Slider.h"
#include "ThreePosButton.h"
#include "LCDDisplay.h"
#include "Remote.h"
#include <LiquidCrystal_I2C.h>

// Définitions des broches
#define LSIDE_L_BTN 22
#define LSIDE_U_BTN 23
#define LSIDE_D_BTN 24
#define LSIDE_R_BTN 25
#define RSIDE_L_BTN 26
#define RSIDE_D_BTN 27
#define DOUBLE_D_BTN 28
#define RSIDE_R_BTN 29
#define RSIDE_U_BTN 30
#define DOUBLE_U_BTN 31

#define JOYSTICK_RIGHT_VERT A1
#define JOYSTICK_RIGHT_HORIZ A0
#define JOYSTICK_RIGHT_SW 34
#define JOYSTICK_LEFT_VERT A11
#define JOYSTICK_LEFT_HORIZ A12
#define JOYSTICK_LEFT_SW 33

#define ENCODER_A 14 // Broche A de l'encodeur (PCINT10)
#define ENCODER_B 15 // Broche B de l'encodeur (PCINT9)
#define ENCODER_SW 32 // Broche du bouton de l'encodeur

#define RF24_CE 48
#define RF24_CSN 49
#define RF24_SCK 52
#define RF24_MISO 50
#define RF24_MOSI 51
#define RF24_IRQ 11

#define THREEPOS_UP 12
#define THREEPOS_DOWN 13

#define SLIDER_PIN A8
#define Motor_Vibr 56

enum EncoderState {
  ENCODER_A_LOW_B_LOW = 0b00,
  ENCODER_A_HIGH_B_LOW = 0b01,
  ENCODER_A_LOW_B_HIGH = 0b10,
  ENCODER_A_HIGH_B_HIGH = 0b11
};


// Variables globales
volatile byte score = 90; // Score initial
byte lastScore = score;
volatile EncoderState lastEncoderState = ENCODER_A_LOW_B_LOW;

// Initialisation des objets
Button buttons[10] = {
  Button(LSIDE_L_BTN), Button(LSIDE_U_BTN), Button(LSIDE_D_BTN),
  Button(LSIDE_R_BTN), Button(RSIDE_L_BTN), Button(RSIDE_U_BTN),
  Button(RSIDE_D_BTN), Button(RSIDE_R_BTN), Button(DOUBLE_U_BTN),
  Button(DOUBLE_D_BTN)
};

Joystick joystickRight(JOYSTICK_RIGHT_VERT, JOYSTICK_RIGHT_HORIZ, JOYSTICK_RIGHT_SW);
Joystick joystickLeft(JOYSTICK_LEFT_VERT, JOYSTICK_LEFT_HORIZ, JOYSTICK_LEFT_SW);
Slider slider(SLIDER_PIN);
ThreePosButton threePosButton(THREEPOS_UP, THREEPOS_DOWN);

LCDDisplay lcd;
Remote remote;

// Fonction d'interruption pour mettre à jour la position de l'encodeur
void updateEncoder() {
  static EncoderState prevState = ENCODER_A_LOW_B_LOW;
  EncoderState currState = (EncoderState)((digitalRead(ENCODER_A) << 1) | digitalRead(ENCODER_B));

  Serial.println("Changed");

  // Détection des crans uniquement sur 00 ou 11
  if ((currState == ENCODER_A_HIGH_B_HIGH || currState == ENCODER_A_LOW_B_LOW) && currState != prevState) {
    // Sens horaire : 00 -> 10 -> 11 ou 11 -> 01 -> 00
    if ((prevState == ENCODER_A_HIGH_B_LOW && currState == ENCODER_A_HIGH_B_HIGH) ||
        (prevState == ENCODER_A_LOW_B_HIGH && currState == ENCODER_A_LOW_B_LOW)) {
      score++;
    }
    // Sens anti-horaire : 00 -> 01 -> 11 ou 11 -> 10 -> 00
    else if ((prevState == ENCODER_A_LOW_B_HIGH && currState == ENCODER_A_HIGH_B_HIGH) ||
             (prevState == ENCODER_A_HIGH_B_LOW && currState == ENCODER_A_LOW_B_LOW)) {
      score--;
    }
    score = constrain(score, 0, 255);
  }

  prevState = currState;
  lastEncoderState = currState;
}

// Configuration des interruptions PCINT
void setupPCINT() {
  cli();
  // Activer les interruptions PCINT pour les broches 14 (PCINT10) et 15 (PCINT9)
  PCMSK1 |= (1 << PCINT10) | (1 << PCINT9); // Activer les interruptions pour les broches 14 et 15
  PCICR |= (1 << PCIE1); // Activer le groupe PCINT1 (broches 8 à 15)

  sei();
}

// Fonction d'interruption PCINT1 (broches 8 à 15)
ISR(PCINT1_vect) {
  updateEncoder(); // Appeler la fonction de mise à jour de l'encodeur
}

void setup() {
  Serial.begin(9600);
  remote.setup();

  pinMode(Motor_Vibr, OUTPUT );

  // Configuration des broches des boutons
  for (int i = 0; i < 10; i++) {
    pinMode(buttons[i].getPin(), INPUT_PULLUP);
  }

  // Configuration des broches des joysticks
  pinMode(JOYSTICK_LEFT_VERT, INPUT);
  pinMode(JOYSTICK_LEFT_HORIZ, INPUT);
  pinMode(JOYSTICK_LEFT_SW, INPUT);

  pinMode(JOYSTICK_RIGHT_VERT, INPUT);
  pinMode(JOYSTICK_RIGHT_HORIZ, INPUT);
  pinMode(JOYSTICK_RIGHT_SW, INPUT);

  // Configuration des broches de l'encodeur
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP); // Broche du bouton de l'encodeur

  // Configuration des broches du bouton à 3 positions
  pinMode(THREEPOS_UP, INPUT);
  pinMode(THREEPOS_DOWN, INPUT);

  // Configuration des broches du slider
  pinMode(SLIDER_PIN, INPUT);

  // Initialisation de la communication RF24
  remote.setup();

  // Initialisation de l'écran LCD
  lcd.init();
  lcd.print("Initialisation"); // Message de démarrage
  lcd.displayScore(score);

  digitalWrite(Motor_Vibr, HIGH); //On allume le moteur
  delay(500); // On fait une pause d'une seconde
  digitalWrite(Motor_Vibr, LOW); // On éteint le moteur

  // Configuration des interruptions PCINT pour l'encodeur
  setupPCINT();


}

void loop() {

  // Mise à jour des capteurs
  for (int i = 0; i < 10; i++) {
    buttons[i].update();
  }
  
  bool currentButtonState = digitalRead(ENCODER_SW); // LOW signifie appuyé (pull-up)

  // // Si le bouton est pressé, ajouter 10 au score
  if (!currentButtonState) {
    score += 5;
    score = constrain(score, 0, 255);
    delay(100);
  }
  

  // // Mettre à jour l'affichage LCD
  if ( lastScore != score) {
    lcd.displayScore(score);
  }
  
  // Préparation des autres données
  RemoteData remoteData;

  // Boutons
  for (int i = 0; i < 10; i++) {
    remoteData.buttons[i] = buttons[i].isHeld();
  }


  // Slider
  remoteData.slider = map(slider.readValue(), 0, 1023, 0, 255);
  remoteData.joystickRight = joystickRight.getData();
  remoteData.joystickLeft = joystickLeft.getData();

  // Score
  lastScore = score; // Utiliser la position de l'encodeur mise à jour par les interruptions

  // Envoi des autres données via RF24
  remoteData.score = score; // Mettre à jour le score dans les données à envoyer
  bool remoteDataSent = remote.sendRemoteData(remoteData);
  if (remoteDataSent) {
    //Serial.println("Autres données envoyées avec succès !");
  } else {
    //Serial.println("Échec de l'envoi des autres données.");
  }

  // Délai pour stabiliser
  delay(20);
}