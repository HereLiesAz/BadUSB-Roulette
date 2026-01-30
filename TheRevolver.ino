/*
   PROJECT: BadUSB Revolver (Unified Edition)
   HARDWARE: ATtiny85 (Digispark)
   
   MODE SELECTION:
   - SINGLE_LED_MODE: Uses the onboard LED. Flashes count, stays SOLID to arm.
   - DUAL_LED_MODE:   Uses two pins. Green flashes count, RED arms.
*/

#include <EEPROM.h>
#include "DigiKeyboard.h"

// --- IMPORT BULLETS ---
#include "src/chamber-one/payload.h"
#include "src/chamber-two/payload.h"
#include "src/chamber-three/payload.h"

// ==========================================
//      CONFIGURATION ZONE
// ==========================================

// UNCOMMENT THIS LINE if you have Red/Green LEDs wired up. 
// LEAVE COMMENTED to use the default onboard LED.
// #define DUAL_LED_MODE 

#define TOTAL_MODES 3
#define SAFE_WINDOW 3000 // Time (ms) to decide before firing
#define MEM_ADDR 0

// --- PIN DEFINITIONS ---
#ifdef DUAL_LED_MODE
  #define PIN_GREEN 0  // Adjust based on your wiring
  #define PIN_RED   1
#else
  #define PIN_MAIN  1  // Default onboard LED (Rev 2 Model A)
#endif

// ==========================================
//      LOGIC ENGINE
// ==========================================

void setup() {
  // 1. PIN SETUP
  #ifdef DUAL_LED_MODE
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_RED, OUTPUT);
    digitalWrite(PIN_GREEN, LOW);
    digitalWrite(PIN_RED, LOW);
  #else
    pinMode(PIN_MAIN, OUTPUT);
    digitalWrite(PIN_MAIN, LOW);
  #endif

  // 2. ROTATION LOGIC (Entropy)
  // Read -> Calculate Next -> Write Immediately
  byte mode = EEPROM.read(MEM_ADDR);
  if (mode >= TOTAL_MODES) mode = 0;
  
  byte nextMode = (mode + 1) % TOTAL_MODES;
  EEPROM.write(MEM_ADDR, nextMode);

  // 3. IDENTIFICATION PHASE (The Count)
  DigiKeyboard.delay(1000); // USB Stabilization

  for (int i = 0; i <= mode; i++) {
    signal_flash(); // Flash the appropriate LED
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
  
  for (;;) {} // Die loop
}

void loop() {}

// ==========================================
//      SIGNALING HELPERS
// ==========================================

void signal_flash() {
  #ifdef DUAL_LED_MODE
    // Green Blink
    digitalWrite(PIN_GREEN, HIGH);
    DigiKeyboard.delay(200);
    digitalWrite(PIN_GREEN, LOW);
  #else
    // Main Blink
    digitalWrite(PIN_MAIN, HIGH);
    DigiKeyboard.delay(200);
    digitalWrite(PIN_MAIN, LOW);
  #endif
}

void signal_arm() {
  #ifdef DUAL_LED_MODE
    // Turn RED to indicate danger
    digitalWrite(PIN_RED, HIGH);
  #else
    // Turn SOLID ON to indicate danger
    digitalWrite(PIN_MAIN, HIGH);
  #endif
}

void signal_fire() {
  #ifdef DUAL_LED_MODE
    // Turn off Red (Go Dark for stealth/power)
    digitalWrite(PIN_RED, LOW);
  #else
    // Turn off Main (Go Dark)
    digitalWrite(PIN_MAIN, LOW);
  #endif
}

void signal_done() {
  #ifdef DUAL_LED_MODE
    // Solid Green = Success
    digitalWrite(PIN_GREEN, HIGH);
  #else
    // Fast Strobe = Success
    while(true) {
      digitalWrite(PIN_MAIN, HIGH);
      DigiKeyboard.delay(50);
      digitalWrite(PIN_MAIN, LOW);
      DigiKeyboard.delay(50);
    }
  #endif
}
