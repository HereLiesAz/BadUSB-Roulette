#ifndef CONFIG_H
#define CONFIG_H

// --- HARDWARE SELECTION ---
// The GitHub Action will toggle this automatically to generate two different .hex files.
// 0 = Single LED Mode (Onboard)
// 1 = Dual LED Mode (Green/Red)
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
