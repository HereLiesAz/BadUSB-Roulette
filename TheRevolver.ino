/*
   PROJECT: BadUSB Revolver (Universal Edition)
   HARDWARE: ATtiny85 (Digispark Rev A & B Compatible)
 */

#include <avr/eeprom.h>
#include "DigiKeyboard.h"
#include "roulette_cfg.h"

// ==========================================
//      VIRTUAL MACHINE DEFINITIONS
// ==========================================
#define OP_END      0x00
#define OP_DELAY    0x01
#define OP_KEY      0x02
#define OP_PRINT    0x03
#define OP_PRINTLN  0x04

// RESERVE 1KB FOR PAYLOADS
// __attribute__((used)) prevents the linker from stripping this symbol
const uint8_t PAYLOAD_STORAGE[1024] PROGMEM __attribute__((used)) = {
  0xCA, 0xFE, 0xBA, 0xBE, 
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00 
};
// ==========================================
//      HARDWARE ABSTRACTION
// ==========================================
void set_led(uint8_t state) {
  #if DUAL_LED_MODE == 1
    // Dual Mode: Respect specific Green/Red pins
    // (Managed by specific signal functions below)
  #else
    // Single/Universal Mode: Blast BOTH pins.
// Covers Model A (Pin 1) and Model B (Pin 0).
    digitalWrite(0, state);
    digitalWrite(1, state);
#endif
}

// ==========================================
//      LOGIC ENGINE
// ==========================================

bool has_payload(byte chamber) {
  if (chamber >= TOTAL_CHAMBERS) return false;
uint16_t offset_loc = 4 + (chamber * 2);
  uint8_t off_h = pgm_read_byte(&PAYLOAD_STORAGE[offset_loc]);
  uint8_t off_l = pgm_read_byte(&PAYLOAD_STORAGE[offset_loc + 1]);
uint16_t ptr = (off_h << 8) | off_l;
  // Pointer must be non-zero and within the storage limit
  return (ptr > 0 && ptr < 1024);
}

void setup() {
  // 1. PANIC BLINK (Hardware Agnostic)
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
for(int k=0; k<5; k++) {
    digitalWrite(0, HIGH);
    digitalWrite(1, HIGH);
    DigiKeyboard.delay(100);
    digitalWrite(0, LOW);
    digitalWrite(1, LOW);
    DigiKeyboard.delay(100);
}

  // 2. STATE RECOVERY
  // Pin modes are already OUTPUT from panic blink.
  // Ensure off state.
digitalWrite(0, LOW);
  digitalWrite(1, LOW);

  // 3. RUSSIAN ROULETTE LOGIC
  byte mode = eeprom_read_byte((const uint8_t*)0);
if (mode >= TOTAL_CHAMBERS) mode = 0;

  // A. Stale Memory Check (Skip empties)
  // Logic: Scan forward until we find a payload or loop back to start
  byte check = mode;
bool found = false;
  for (int i = 0; i < TOTAL_CHAMBERS; i++) {
    if (has_payload(check)) {
        found = true;
mode = check;
        break;
    }
    check = (check + 1) % TOTAL_CHAMBERS;
}

  // FAILSAFE: If ALL chambers are empty, die SOS.
if (!found) {
    while(1) {
      set_led(HIGH);
      DigiKeyboard.delay(100);
      set_led(LOW);
      DigiKeyboard.delay(100);
}
  }

  // B. Calculate & Write NEXT Step Immediately
  // If user unplugs during ARMING, this value persists.
byte next = (mode + 1) % TOTAL_CHAMBERS;
  // Ensure the next one we queue is also valid (so we don't boot into a dead chamber)
  for (int i = 0; i < TOTAL_CHAMBERS; i++) {
    if (has_payload(next)) break;
next = (next + 1) % TOTAL_CHAMBERS;
  }
  eeprom_update_byte((uint8_t*)0, next);

  // 4. IDENTIFICATION PHASE
  DigiKeyboard.delay(1000);
for (int i = 0; i <= mode; i++) {
    signal_flash();
    DigiKeyboard.delay(300);
}

  // 5. ARMING PHASE
  signal_arm(); 
  DigiKeyboard.delay(SAFE_WINDOW);

  // 6. FIRE PHASE
  signal_fire();
// EXECUTE PAYLOAD
  uint16_t offset_loc = 4 + (mode * 2);
  uint8_t off_h = pgm_read_byte(&PAYLOAD_STORAGE[offset_loc]);
uint8_t off_l = pgm_read_byte(&PAYLOAD_STORAGE[offset_loc + 1]);
  uint16_t payload_addr = (off_h << 8) | off_l;
if (payload_addr > 0 && payload_addr < 1024) {
     run_vm(payload_addr);
}

  // 7. COMPLETION PHASE
  signal_done();
  for (;;) {
    DigiKeyboard.delay(1000); // Keep USB alive
  } 
}

void loop() {}

// ==========================================
//      BYTECODE INTERPRETER
// ==========================================
void run_vm(uint16_t ptr) {
  while(true) {
    uint8_t opcode = pgm_read_byte(&PAYLOAD_STORAGE[ptr++]);
    
    // Safety: 0xFF is erased flash memory. Treat as end.
if (opcode == OP_END || opcode == 0xFF) break;
    
    else if (opcode == OP_DELAY) {
      uint8_t h = pgm_read_byte(&PAYLOAD_STORAGE[ptr++]);
uint8_t l = pgm_read_byte(&PAYLOAD_STORAGE[ptr++]);
      uint16_t t = (h << 8) | l;
      DigiKeyboard.delay(t);
}
    
    else if (opcode == OP_KEY) {
      uint8_t mod = pgm_read_byte(&PAYLOAD_STORAGE[ptr++]);
uint8_t key = pgm_read_byte(&PAYLOAD_STORAGE[ptr++]);
      DigiKeyboard.sendKeyStroke(key, mod);
    }
    
    else if (opcode ==
