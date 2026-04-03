#include "Button.h"

Button::Button(int p) : button(p), pin(p) {
  button.setDebounceTime(0); // Débounce de 50 ms
}

void Button::update() {
  button.loop(); // Mise à jour de l'état du bouton avec debounce
}

bool Button::isPressed() {
  return button.isPressed(); // Retourne true si le bouton a été pressé
}

bool Button::isHeld() {
  return button.getState() == LOW; // Avec INPUT_PULLUP, LOW signifie appuye
}

int Button::getPin() {
  return pin; // Retourne le numéro de broche
}