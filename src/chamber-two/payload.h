#ifndef BULLET_2_H
#define BULLET_2_H

#include "DigiKeyboard.h"

void fire_2() {
  // --- BULLET 2: MAC TARGET ---
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.sendKeyStroke(KEY_SPACE, MOD_GUI_LEFT);
  DigiKeyboard.delay(500);
  DigiKeyboard.print("Terminal");
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
}

#endif
