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

constexpr uint8_t RF_FREQ_HZ = 50;

const uint8_t LOOP_DELAY_MS = 1000 / RF_FREQ_HZ;

enum EncoderState {
  ENCODER_A_LOW_B_LOW = 0b00,
  ENCODER_A_HIGH_B_LOW = 0b01,
  ENCODER_A_LOW_B_HIGH = 0b10,
  ENCODER_A_HIGH_B_HIGH = 0b11
};

bool isYellow = false;
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
  //updateEncoder(); // Appeler la fonction de mise à jour de l'encodeur
}

void setup() {
  Serial.begin(9600);
  
  // Initialisation de l'écran LCD
  lcd.init();
  lcd.print("Initialisation..."); // Message de démarrage

  remote.setup();

  pinMode(Motor_Vibr, OUTPUT );
  for (int i = 0; i < 10; i++) {
    pinMode(buttons[i].getPin(), INPUT_PULLUP);
  }

  pinMode(JOYSTICK_LEFT_VERT, INPUT);
  pinMode(JOYSTICK_LEFT_HORIZ, INPUT);
  pinMode(JOYSTICK_LEFT_SW, INPUT);

  pinMode(JOYSTICK_RIGHT_VERT, INPUT);
  pinMode(JOYSTICK_RIGHT_HORIZ, INPUT);
  pinMode(JOYSTICK_RIGHT_SW, INPUT);

  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);

  pinMode(THREEPOS_UP, INPUT);
  pinMode(THREEPOS_DOWN, INPUT);

  pinMode(SLIDER_PIN, INPUT);

  remote.setup();

  setupPCINT();
  
  digitalWrite(Motor_Vibr, HIGH);
  delay(500);
  digitalWrite(Motor_Vibr, LOW);
  
  lcd.print(isYellow ? "Team Yellow" : "Team Blue");
}

void loop() {

  // Mise à jour des capteurs
  for (int i = 0; i < 10; i++) {
    buttons[i].update();
  }

  if (buttons[5].isPressed()) {
    if (!isYellow) {
      isYellow = true;
      lcd.print("Team Yellow");
    }
  } else if (buttons[4].isPressed()) {
    if (isYellow) {
      isYellow = false;
      lcd.print("Team Blue");
    }
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

  // Envoi des autres données via RF24
  remoteData.isYellow = isYellow; // Mettre à jour la couleur dans les données à envoyer
  remote.sendRemoteData(remoteData);

  // Délai pour stabiliser
  delay(LOOP_DELAY_MS);
}