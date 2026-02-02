#ifndef ROULETTE_CFG_H
#define ROULETTE_CFG_H

// --- HARDWARE CONFIG ---
// DUAL_LED_MODE 0 = Universal Single (Blinks P0 & P1)
// DUAL_LED_MODE 1 = Dual Mode (Green P0, Red P1)
#define DUAL_LED_MODE 0 

// --- PIN MAPPING ---
// In Universal Mode (0), these defines are ignored by the wrapper.
// In Dual Mode (1), specific pins are used.
#define PIN_GREEN  0
#define PIN_RED    1

// --- GAME PHYSICS ---
#define TOTAL_CHAMBERS 3
#define SAFE_WINDOW 3000

#endif
