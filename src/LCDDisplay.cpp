#include "LCDDisplay.h"

LCDDisplay::LCDDisplay() : lcd(0x6B, 16, 2) {}

void LCDDisplay::init() {
  lcd.init(); // Initialisation de l'écran LCD
  lcd.setPWM(BLUE, 254);
}

void LCDDisplay::print(const char* text) {
  lcd.clear(); // Efface l'écran avant d'afficher le nouveau texte
  lcd.setCursor(0, 0); // Positionne le curseur sur la première ligne
  lcd.print(text); // Affichage d'un texte sur l'écran
}

void LCDDisplay::printTeamAndSpeed(bool isYellow, uint8_t speedPercent) {
  lcd.clear(); // Efface l'écran avant d'afficher le nouveau texte
  
  // Ligne 1 : Équipe
  lcd.setCursor(0, 0);
  lcd.print("Team : ");
  lcd.print(isYellow ? "Yellow" : "Blue");
  
  // Ligne 2 : Vitesse
  lcd.setCursor(0, 1);
  lcd.print("Speed : ");
  lcd.print(speedPercent);
  lcd.print("%");
}