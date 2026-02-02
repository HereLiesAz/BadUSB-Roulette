/*
   PROJECT: BadUSB Revolver (Unified Edition)
   HARDWARE: ATtiny85 (Digispark)
   
   MODE SELECTION:
   - SINGLE_LED_MODE: Uses the onboard LED.
     Flashes count, stays SOLID to arm.
   - DUAL_LED_MODE:   Uses two pins. Green flashes count, RED arms.
 */

#include <EEPROM.h>
#include "DigiKeyboard.h"
#include "config.h" // FIXED: Actually import the settings

// --- IMPORT BULLETS ---
#include "src/chamber-one/payload.h"
#include "src/chamber-two/payload.h"
#include "src/chamber-three/payload.h"

// ==========================================
//      CONFIGURATION ZONE
// ==========================================
// Settings are now handled strictly in config.h 
// or overridden by the GitHub Action via sed.

// ==========================================
//      LOGIC ENGINE
// ==========================================

void setup() {
  // 1. PIN SETUP
  #if DUAL_LED_MODE == 1
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_RED, OUTPUT);
    digitalWrite(PIN_GREEN, LOW);
    digitalWrite(PIN_RED, LOW);
  #else
    pinMode(PIN_SINGLE, OUTPUT); // Fixed variable name from config.h
    digitalWrite(PIN_SINGLE, LOW);
  #endif

  // 2. ROTATION LOGIC (Entropy)
  // Read -> Calculate Next -> Write Immediately
  byte mode = EEPROM.read(0);
  if (mode >= TOTAL_CHAMBERS) mode = 0;
  
  byte nextMode = (mode + 1) % TOTAL_CHAMBERS;
  EEPROM.write(0, nextMode);

  // 3. IDENTIFICATION PHASE (The Count)
  DigiKeyboard.delay(1000); // USB Stabilization

  for (int i = 0; i <= mode; i++) {
    signal_flash();
    DigiKeyboard.delay(300);
  }

  // 4. ARMING PHASE (The Warning)
  // This is the window to pull out.
  signal_arm(); 
  DigiKeyboard.delay(SAFE_WINDOW);

  // 5. FIRE PHASE
  signal_fire(); // Go dark/busy
  
  switch (mode) {
    case 0: fire_windows(); break;
    case 1: fire_mac(); break;
    case 2: fire_nix(); break;
  }

  // 6. COMPLETION PHASE
  signal_done();
  for (;;) {} // Universal Die loop
}

void loop() {}

// ==========================================
//      SIGNALING HELPERS
// ==========================================

void signal_flash() {
  #if DUAL_LED_MODE == 1
    // Green Blink
    digitalWrite(PIN_GREEN, HIGH);
    DigiKeyboard.delay(200);
    digitalWrite(PIN_GREEN, LOW);
  #else
    // Main Blink
    digitalWrite(PIN_SINGLE, HIGH);
    DigiKeyboard.delay(200);
    digitalWrite(PIN_SINGLE, LOW);
  #endif
}

void signal_arm() {
  #if DUAL_LED_MODE == 1
    // Turn RED to indicate danger
    digitalWrite(PIN_RED, HIGH);
  #else
    // Turn SOLID ON to indicate danger
    digitalWrite(PIN_SINGLE, HIGH);
  #endif
}

void signal_fire() {
  #if DUAL_LED_MODE == 1
    // Turn off Red (Go Dark for stealth/power)
    digitalWrite(PIN_RED, LOW);
  #else
    // Turn off Main (Go Dark)
    digitalWrite(PIN_SINGLE, LOW);
  #endif
}

void signal_done() {
  #if DUAL_LED_MODE == 1
    // Solid Green = Success
    digitalWrite(PIN_GREEN, HIGH);
  #else
    // Fast Strobe = Success
    // We do not loop here; we let the main setup() hit the eternal loop.
    for(int i=0; i<10; i++){
       digitalWrite(PIN_SINGLE, HIGH);
       DigiKeyboard.delay(50);
       digitalWrite(PIN_SINGLE, LOW);
       DigiKeyboard.delay(50);
    }
    digitalWrite(PIN_SINGLE, HIGH); // Leave on to signify done
  #endif
}
