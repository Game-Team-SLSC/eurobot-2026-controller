#include "ThreePosButton.h"

ThreePosButton::ThreePosButton(int pU, int pD) : buttonUp(pU), buttonDown(pD), state(SWITCH_3_POS::MIDDLE) {
  buttonUp.setDebounceTime(50);   // Débounce de 50 ms
  buttonDown.setDebounceTime(50); // Débounce de 50 ms
}

void ThreePosButton::update() {
  buttonUp.loop();   // Mise à jour de l'état du bouton Up avec debounce
  buttonDown.loop(); // Mise à jour de l'état du bouton Down avec debounce

  if (buttonUp.isPressed()) {
    state = SWITCH_3_POS::UP;
  } else if (buttonDown.isPressed()) {
    state = SWITCH_3_POS::DOWN;
  } else {
    state = SWITCH_3_POS::MIDDLE; // Si aucun bouton n'est pressé, on considère que le bouton est au milieu
  }
}

SWITCH_3_POS ThreePosButton::getState() {
  return state; // Retourne l'état du bouton à 3 positions
}