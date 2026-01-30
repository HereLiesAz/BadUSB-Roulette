#ifndef BULLET_1_H
#define BULLET_1_H

#include "DigiKeyboard.h"

void fire_1() {
  // --- BULLET 1: WINDOWS TARGET ---
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.delay(500);
  DigiKeyboard.sendKeyStroke(KEY_R, MOD_GUI_LEFT);
  DigiKeyboard.delay(500);
  DigiKeyboard.print("powershell -w h Start-Process notepad"); // Example
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
}

#endif
