#ifndef ROULETTE_CFG_H
#define ROULETTE_CFG_H
/**
 * @file roulette_cfg.h
 * @brief Configuration constants for the BadUSB-Roulette firmware.
 *
 * This file controls the hardware behavior, pin mappings, and game mechanics.
 * It is included by the main firmware file (TheRevolver.ino).
 */

// --- HARDWARE CONFIG ---

/**
 * @brief DUAL_LED_MODE determines the hardware revision.
 *
 * 0: Universal Single Mode (Default).
 *    - Uses both Pin 0 and Pin 1 for all LED signals.
 *    - Compatible with Digispark Model A and Model B.
 *
 * 1: Dual LED Mode (Custom Hardware).
 *    - Uses specific pins for Green (Safe) and Red (Danger) signals.
 */
#define DUAL_LED_MODE 0 

// --- PIN MAPPING ---

/**
 * @brief Pin definitions for Dual LED Mode.
 *
 * If DUAL_LED_MODE is 0, these values are ignored by the wrapper function `set_led`,
 * which instead toggles both Pin 0 and Pin 1 simultaneously.
 */
#define PIN_GREEN  0  // Pin connected to the Green LED (Safe/Idle)
#define PIN_RED    1  // Pin connected to the Red LED (Armed/Fire)

// --- GAME PHYSICS ---

/**
 * @brief The number of available payload slots ("chambers").
 *
 * The firmware will cycle through these slots using EEPROM to store the state.
 * The payload storage area must be large enough to hold offsets for all chambers.
 * (Header size = 4 + (TOTAL_CHAMBERS * 2) bytes).
 */
#define TOTAL_CHAMBERS 3

/**
 * @brief The duration (in milliseconds) the device waits before executing the payload.
 *
 * During this window, the LED will be solid (Armed).
 * If the user unplugs the device during this window, the payload is skipped.
 * If the user waits, the payload executes ("Fires").
 */
#define SAFE_WINDOW 3000

#endif // ROULETTE_CFG_H
