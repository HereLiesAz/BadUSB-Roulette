#ifndef ROULETTE_CFG_H
#define ROULETTE_CFG_H

// --- HARDWARE SELECTION ---
// This file is manipulated by the GitHub Action to generate
// two distinct firmware templates (Single and Dual).
#define DUAL_LED_MODE 0 

// --- PINS ---
// Digispark Rev 2 (Model A) LED is Pin 1
// Digispark Rev 1 (Model B) LED is Pin 0
#define PIN_SINGLE 1
#define PIN_GREEN  0
#define PIN_RED    1

// --- TIMINGS ---
#define TOTAL_CHAMBERS 3
#define SAFE_WINDOW 3000

#endif
