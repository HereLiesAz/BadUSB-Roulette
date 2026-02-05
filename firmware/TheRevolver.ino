/**
 * @file TheRevolver.ino
 * @brief Main firmware for the BadUSB-Roulette device.
 *
 * This firmware implements a Virtual Machine (VM) that executes custom bytecode payloads.
 * It manages the "Russian Roulette" game logic, cycling through payloads stored in Flash memory
 * and executing them based on user interaction (plug/unplug/wait).
 *
 * Supported Hardware: Digispark ATTiny85 (Rev A & B)
 */

#include <avr/eeprom.h>     // Required for reading/writing non-volatile memory (EEPROM).
#include "DigiKeyboard.h"   // Library for USB HID Keyboard emulation.
#include "roulette_cfg.h"   // Configuration file for hardware pins and game settings.

// ==========================================
//      VIRTUAL MACHINE DEFINITIONS
// ==========================================

// Opcode Definitions for the custom bytecode interpreter.
// These byte values correspond to specific actions the VM can take.
#define OP_END      0x00    // Terminates execution of the current payload.
#define OP_DELAY    0x01    // Pauses execution for a specified duration (followed by 2-byte ms).
#define OP_KEY      0x02    // Sends a keystroke (followed by Modifier byte + Key byte).
#define OP_PRINT    0x03    // Types a string (followed by Length byte + characters).
#define OP_PRINTLN  0x04    // Types a string followed by ENTER (Length + chars).

// ==========================================
//      PAYLOAD STORAGE
// ==========================================

/**
 * @brief Reserves 1KB of Flash memory for storing payloads.
 *
 * We use `PROGMEM` to store this array in Flash (Program Memory) rather than RAM,
 * as the ATTiny85 has very limited RAM (512 bytes).
 *
 * `__attribute__((used))` tells the compiler/linker to keep this array in the binary
 * even if it appears unused in the code (which prevents optimization removal).
 *
 * Structure:
 * - Bytes 0-3: Magic Header (0xCA, 0xFE, 0xBA, 0xBE) to verify valid firmware patching.
 * - Bytes 4-9: 16-bit offsets for Chamber 1, 2, and 3 pointers.
 * - Bytes 10+: Actual bytecode data for the payloads.
 */
const uint8_t PAYLOAD_STORAGE[1024] PROGMEM __attribute__((used)) = {
  0xCA, 0xFE, 0xBA, 0xBE, // Magic Header: CAFEBABE
  0x00, 0x00, 0x00, 0x00, // Placeholders for Chamber 1 & 2 Offsets
  0x00, 0x00              // Placeholder for Chamber 3 Offset
  // The rest is zero-initialized by the compiler.
};

// ==========================================
//      FORWARD DECLARATIONS
// ==========================================
// Declare functions before use to satisfy C++ compiler requirements.
void set_led(uint8_t state);    // Controls the onboard LED(s).
bool has_payload(byte chamber); // Checks if a specific chamber has a valid payload.
void run_vm(uint16_t ptr);      // Executes the bytecode starting at a memory address.
void signal_flash();            // Blinks the LED to indicate chamber selection.
void signal_arm();              // Turns the LED on to indicate the "Armed" state.
void signal_fire();             // Turns the LED off to indicate execution ("Fire").
void signal_done();             // Blinks rapidly to indicate completion.

// ==========================================
//      HARDWARE ABSTRACTION
// ==========================================

/**
 * @brief Controls the device's LED(s) based on the configuration mode.
 *
 * @param state HIGH (on) or LOW (off).
 */
void set_led(uint8_t state) {
  #if DUAL_LED_MODE == 1
    // DUAL_LED_MODE 1: Dual LED Configuration.
    // In this mode, we do not control pins generically here.
    // Instead, specific signal functions (signal_arm, signal_fire) manage
    // the RED (Danger) and GREEN (Safe) pins directly.
  #else
    // DUAL_LED_MODE 0: Universal Single LED Mode.
    // Digispark Rev A uses Pin 1 for LED. Rev B uses Pin 0.
    // To be universal, we toggle BOTH pins simultaneously.
    digitalWrite(0, state); // Toggle Pin 0 (Model B LED)
    digitalWrite(1, state); // Toggle Pin 1 (Model A LED)
  #endif
}

// ==========================================
//      LOGIC ENGINE
// ==========================================

/**
 * @brief Checks if a specific chamber contains a valid payload.
 *
 * It reads the offset from the PAYLOAD_STORAGE header and verifies it points
 * to a valid location within the storage array.
 *
 * @param chamber The index of the chamber to check (0 to TOTAL_CHAMBERS-1).
 * @return true if a valid payload exists, false otherwise.
 */
bool has_payload(byte chamber) {
  // Safety check: Ensure chamber index is within bounds.
  if (chamber >= TOTAL_CHAMBERS) return false;

  // Calculate the memory location of the offset for this chamber.
  // Header is 4 bytes. Each offset is 2 bytes.
  // Location = 4 + (chamber_index * 2)
  uint16_t offset_loc = 4 + (chamber * 2);

  // Read the 16-bit offset from PROGMEM (Flash memory).
  // Offsets are stored Big-Endian (High Byte, Low Byte).
  uint8_t off_h = pgm_read_byte(&PAYLOAD_STORAGE[offset_loc]);
  uint8_t off_l = pgm_read_byte(&PAYLOAD_STORAGE[offset_loc + 1]);

  // Combine bytes into a 16-bit pointer.
  uint16_t ptr = (off_h << 8) | off_l;

  // Validate the pointer:
  // 1. Must be > 0 (0 indicates empty).
  // 2. Must be < 1024 (must point within the storage array).
  return (ptr > 0 && ptr < 1024);
}

/**
 * @brief Standard Arduino Setup function. Runs once at boot.
 *
 * This function contains the entire "Game Loop" logic. Since the device acts as a
 * single-shot payload deliverer, we do not use the main loop().
 */
void setup() {
  // 1. PANIC BLINK (Hardware Agnostic Initialization)
  // Initialize pins 0 and 1 as outputs immediately.
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);

  // Rapidly blink 5 times at startup to indicate power-on / bootloader exit.
  for(int k=0; k<5; k++) {
    digitalWrite(0, HIGH); // LED ON
    digitalWrite(1, HIGH);
    DigiKeyboard.delay(100); // Wait 100ms
    digitalWrite(0, LOW);  // LED OFF
    digitalWrite(1, LOW);
    DigiKeyboard.delay(100); // Wait 100ms
  }

  // 2. STATE RECOVERY
  // Ensure LEDs are off after the panic blink.
  digitalWrite(0, LOW);
  digitalWrite(1, LOW);

  // 3. RUSSIAN ROULETTE LOGIC

  // Read the last saved state (current chamber index) from EEPROM address 0.
  byte mode = eeprom_read_byte((const uint8_t*)0);

  // Sanity check: If EEPROM value is garbage (e.g., 255 from factory), reset to 0.
  if (mode >= TOTAL_CHAMBERS) mode = 0;

  // A. Stale Memory Check (Skip Empty Chambers)
  // We need to find the next *valid* chamber starting from the saved 'mode'.
  byte check = mode;
  bool found = false;

  // Loop through all chambers to find one with a payload.
  for (int i = 0; i < TOTAL_CHAMBERS; i++) {
    if (has_payload(check)) {
        found = true;   // Valid payload found.
        mode = check;   // Update current mode to this valid chamber.
        break;
    }
    // Move to the next chamber, wrapping around if necessary.
    check = (check + 1) % TOTAL_CHAMBERS;
  }

  // FAILSAFE: If NO chambers have payloads (empty device), enter an infinite SOS loop.
  // This prevents the device from doing nothing and confusing the user.
  if (!found) {
    while(1) {
      set_led(HIGH);        // LED ON
      DigiKeyboard.delay(100);
      set_led(LOW);         // LED OFF
      DigiKeyboard.delay(100);
    }
  }

  // B. Calculate & Write NEXT Step Immediately (Advance the Barrel)
  // We update the state in EEPROM *before* executing.
  // This ensures that if the user unplugs the device now (during the "Safe Window"),
  // the device will boot into the *next* chamber next time.
  byte next = (mode + 1) % TOTAL_CHAMBERS;

  // Ensure the *next* state we write is also a valid chamber (skip empties for next boot too).
  for (int i = 0; i < TOTAL_CHAMBERS; i++) {
    if (has_payload(next)) break; // Found a valid next chamber.
    next = (next + 1) % TOTAL_CHAMBERS;
  }

  // Save the next chamber index to EEPROM.
  eeprom_update_byte((uint8_t*)0, next);

  // 4. IDENTIFICATION PHASE
  // Wait 1 second before identifying, to let the user settle the device.
  DigiKeyboard.delay(1000);

  // Blink the LED 'mode + 1' times to tell the user which chamber is active.
  // (e.g., Chamber 0 blinks 1 time, Chamber 1 blinks 2 times).
  for (int i = 0; i <= mode; i++) {
    signal_flash();       // Blink ON
    DigiKeyboard.delay(300); // Wait between blinks
  }

  // 5. ARMING PHASE (The Safe Window)
  // Turn the LED on solid (Red if dual mode) to warn the user execution is imminent.
  signal_arm(); 

  // Wait for the SAFE_WINDOW duration (e.g., 3 seconds).
  // This gives the user time to unplug the device if they don't want to "fire" this payload.
  DigiKeyboard.delay(SAFE_WINDOW);

  // 6. FIRE PHASE (Execution)
  // If execution reaches here, the user did not unplug.

  signal_fire(); // Turn off LED (or signal firing).

  // Retrieve the pointer to the payload again.
  uint16_t offset_loc = 4 + (mode * 2);
  uint8_t off_h = pgm_read_byte(&PAYLOAD_STORAGE[offset_loc]);
  uint8_t off_l = pgm_read_byte(&PAYLOAD_STORAGE[offset_loc + 1]);
  uint16_t payload_addr = (off_h << 8) | off_l;

  // Double-check validity before running VM.
  if (payload_addr > 0 && payload_addr < 1024) {
     run_vm(payload_addr); // Start the Virtual Machine.
  }

  // 7. COMPLETION PHASE
  // Signal that the payload has finished executing.
  signal_done();

  // Enter an infinite loop to keep the USB connection alive but do nothing else.
  // DigiKeyboard.delay is used to keep the USB interrupt poll active.
  for (;;) {
    DigiKeyboard.delay(1000);
  } 
}

/**
 * @brief Standard Arduino Loop.
 *
 * Unused in this architecture because setup() handles the single-pass logic and never returns.
 */
void loop() {}

// ==========================================
//      BYTECODE INTERPRETER (VM)
// ==========================================

/**
 * @brief Interprets and executes the bytecode payload.
 *
 * @param ptr The starting address of the bytecode in PROGMEM (Flash).
 */
void run_vm(uint16_t ptr) {
  while(true) {
    // Fetch the next opcode from Flash memory.
    uint8_t opcode = pgm_read_byte(&PAYLOAD_STORAGE[ptr++]);
    
    // Check for Termination Opcodes.
    // OP_END (0x00) is the standard end.
    // 0xFF is the default value of erased flash memory (safety catch).
    if (opcode == OP_END || opcode == 0xFF) break;
    
    // --- OP_DELAY (0x01) ---
    // Format: [0x01] [High Byte] [Low Byte]
    else if (opcode == OP_DELAY) {
      uint8_t h = pgm_read_byte(&PAYLOAD_STORAGE[ptr++]); // Read High Byte
      uint8_t l = pgm_read_byte(&PAYLOAD_STORAGE[ptr++]); // Read Low Byte
      uint16_t t = (h << 8) | l; // Combine into 16-bit milliseconds
      DigiKeyboard.delay(t);     // Execute delay
    }
    
    // --- OP_KEY (0x02) ---
    // Format: [0x02] [Modifiers] [Key Code]
    else if (opcode == OP_KEY) {
      uint8_t mod = pgm_read_byte(&PAYLOAD_STORAGE[ptr++]); // Read Modifiers
      uint8_t key = pgm_read_byte(&PAYLOAD_STORAGE[ptr++]); // Read Key Code
      DigiKeyboard.sendKeyStroke(key, mod); // Send keystroke via HID
    }
    
    // --- OP_PRINT (0x03) & OP_PRINTLN (0x04) ---
    // Format: [Opcode] [Length] [Char 1] [Char 2] ...
    else if (opcode == OP_PRINT || opcode == OP_PRINTLN) {
      uint8_t len = pgm_read_byte(&PAYLOAD_STORAGE[ptr++]); // Read String Length

      // Loop through the string data
      for (int i=0; i<len; i++) {
        char c = (char)pgm_read_byte(&PAYLOAD_STORAGE[ptr++]); // Read character
        DigiKeyboard.print(c); // Type character via HID
      }

      // If it's PRINTLN, append a newline (Enter).
      if (opcode == OP_PRINTLN) DigiKeyboard.print("\n");
    }
    
    // Safety Break: Prevent reading beyond the 1KB storage limit.
    if (ptr >= 1024) break;
  }

  // CLEANUP: Ensure no keys are left logically "pressed" after execution.
  DigiKeyboard.sendKeyStroke(0);
}

// ==========================================
//      SIGNALING HELPERS
// ==========================================

/**
 * @brief Blinks the LED to indicate presence/identification.
 *
 * Flashes GREEN in Dual Mode, or standard blink in Universal Mode.
 */
void signal_flash() {
  #if DUAL_LED_MODE == 1
    // Dual Mode: Flash Green LED.
    digitalWrite(PIN_GREEN, HIGH);
    DigiKeyboard.delay(200);
    digitalWrite(PIN_GREEN, LOW);
  #else
    // Universal Mode: Use generic set_led.
    set_led(HIGH);
    DigiKeyboard.delay(200);
    set_led(LOW);
  #endif
}

/**
 * @brief Sets the LED to the "Armed" state (Solid On).
 *
 * Turns on RED LED in Dual Mode to indicate danger.
 */
void signal_arm() {
  #if DUAL_LED_MODE == 1
    // Dual Mode: Turn on Red LED.
    digitalWrite(PIN_RED, HIGH);
  #else
    // Universal Mode: Turn on standard LEDs.
    set_led(HIGH);
  #endif
}

/**
 * @brief Sets the LED to the "Fire" state (Off).
 *
 * Turns off the RED LED to indicate the safe window has passed and execution is starting.
 */
void signal_fire() {
  #if DUAL_LED_MODE == 1
    // Dual Mode: Turn off Red LED.
    digitalWrite(PIN_RED, LOW);
  #else
    // Universal Mode: Turn off standard LEDs.
    set_led(LOW);
  #endif
}

/**
 * @brief Signals completion of the payload execution.
 *
 * Dual Mode: Turns on Green LED solid.
 * Universal Mode: Rapid blink sequence.
 */
void signal_done() {
  #if DUAL_LED_MODE == 1
    // Dual Mode: Solid Green indicates success/done.
    digitalWrite(PIN_GREEN, HIGH);
  #else
    // Universal Mode: Rapid 5-blink sequence.
    for(int i=0; i<5; i++){
       set_led(HIGH);
       DigiKeyboard.delay(50);
       set_led(LOW);
       DigiKeyboard.delay(50);
    }
  #endif
}
