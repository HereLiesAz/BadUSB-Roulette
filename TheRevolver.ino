/*
   PROJECT: BadUSB Revolver (Web-Flasher Edition)
   HARDWARE: ATtiny85 (Digispark)
 */

#include <avr/eeprom.h>
#include "DigiKeyboard.h"
#include "config.h"

// ==========================================
//      VIRTUAL MACHINE DEFINITIONS
// ==========================================
#define OP_END      0x00
#define OP_DELAY    0x01
#define OP_KEY      0x02
#define OP_PRINT    0x03
#define OP_PRINTLN  0x04

// RESERVE 1KB FOR PAYLOADS
// Header Structure (10 Bytes):
// [0-3] MAGIC (CAFEBABE)
// [4-5] Chamber 1 Offset
// [6-7] Chamber 2 Offset
// [8-9] Chamber 3 Offset
const uint8_t PAYLOAD_STORAGE[1024] PROGMEM = {
  0xCA, 0xFE, 0xBA, 0xBE, 
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00 
};

// ==========================================
//      LOGIC ENGINE
// ==========================================

// Helper: Check if a chamber has valid data (Offset != 0)
bool has_payload(byte chamber) {
  if (chamber >= TOTAL_CHAMBERS) return false;
  
  // Calculate offset location in the header
  // Chamber 0 -> Index 4
  // Chamber 1 -> Index 6
  // Chamber 2 -> Index 8
  uint16_t offset_loc = 4 + (chamber * 2);
  
  uint8_t off_h = pgm_read_byte(&PAYLOAD_STORAGE[offset_loc]);
  uint8_t off_l = pgm_read_byte(&PAYLOAD_STORAGE[offset_loc + 1]);
  uint16_t ptr = (off_h << 8) | off_l;
  
  return (ptr != 0);
}

void setup() {
  // 1. PIN SETUP
  #if DUAL_LED_MODE == 1
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_RED, OUTPUT);
    digitalWrite(PIN_GREEN, LOW);
    digitalWrite(PIN_RED, LOW);
  #else
    pinMode(PIN_SINGLE, OUTPUT);
    digitalWrite(PIN_SINGLE, LOW);
  #endif

  // 2. SMART ROTATION LOGIC
  byte mode = eeprom_read_byte((const uint8_t*)0);
  if (mode >= TOTAL_CHAMBERS) mode = 0;

  // A. Stale Memory Check
  // If the current mode points to an empty chamber, scan forward immediately.
  for (int i = 0; i < TOTAL_CHAMBERS; i++) {
    if (has_payload(mode)) break;
    mode = (mode + 1) % TOTAL_CHAMBERS;
  }

  // FAILSAFE: If ALL chambers are empty, die here.
  if (!has_payload(mode)) {
    while(1) {
      signal_arm(); DigiKeyboard.delay(100);
      signal_fire(); DigiKeyboard.delay(100);
    }
  }

  // B. Calculate NEXT Step (Skip empty chambers for next boot)
  byte next = (mode + 1) % TOTAL_CHAMBERS;
  for (int i = 0; i < TOTAL_CHAMBERS; i++) {
    if (has_payload(next)) break;
    next = (next + 1) % TOTAL_CHAMBERS;
  }
  
  // Write the future immediately
  eeprom_update_byte((uint8_t*)0, next);

  // 3. IDENTIFICATION PHASE
  DigiKeyboard.delay(1000); 

  // Flash the count (1-based index for humans)
  for (int i = 0; i <= mode; i++) {
    signal_flash();
    DigiKeyboard.delay(300);
  }

  // 4. ARMING PHASE
  signal_arm(); 
  DigiKeyboard.delay(SAFE_WINDOW);

  // 5. FIRE PHASE
  signal_fire();
  
  // EXECUTE PAYLOAD
  uint16_t offset_loc = 4 + (mode * 2);
  uint8_t off_h = pgm_read_byte(&PAYLOAD_STORAGE[offset_loc]);
  uint8_t off_l = pgm_read_byte(&PAYLOAD_STORAGE[offset_loc + 1]);
  uint16_t payload_addr = (off_h << 8) | off_l;
  
  if (payload_addr > 0 && payload_addr < 1024) {
     run_vm(payload_addr);
  }

  // 6. COMPLETION PHASE
  signal_done();
  for (;;) {} 
}

void loop() {}

// ==========================================
//      BYTECODE INTERPRETER
// ==========================================
void run_vm(uint16_t ptr) {
  while(true) {
    uint8_t opcode = pgm_read_byte(&PAYLOAD_STORAGE[ptr++]);

    if (opcode == OP_END) break;
    
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
    
    else if (opcode == OP_PRINT || opcode == OP_PRINTLN) {
      uint8_t len = pgm_read_byte(&PAYLOAD_STORAGE[ptr++]);
      for (int i=0; i<len; i++) {
        char c = (char)pgm_read_byte(&PAYLOAD_STORAGE[ptr++]);
        DigiKeyboard.print(c);
      }
      if (opcode == OP_PRINTLN) DigiKeyboard.print("\n");
    }
    
    if (ptr >= 1024) break;
  }
}

// ==========================================
//      SIGNALING HELPERS
// ==========================================

void signal_flash() {
  #if DUAL_LED_MODE == 1
    digitalWrite(PIN_GREEN, HIGH);
    DigiKeyboard.delay(200);
    digitalWrite(PIN_GREEN, LOW);
  #else
    digitalWrite(PIN_SINGLE, HIGH);
    DigiKeyboard.delay(200);
    digitalWrite(PIN_SINGLE, LOW);
  #endif
}

void signal_arm() {
  #if DUAL_LED_MODE == 1
    digitalWrite(PIN_RED, HIGH);
  #else
    digitalWrite(PIN_SINGLE, HIGH);
  #endif
}

void signal_fire() {
  #if DUAL_LED_MODE == 1
    digitalWrite(PIN_RED, LOW);
  #else
    digitalWrite(PIN_SINGLE, LOW);
  #endif
}

void signal_done() {
  #if DUAL_LED_MODE == 1
    digitalWrite(PIN_GREEN, HIGH);
  #else
    for(int i=0; i<10; i++){
       digitalWrite(PIN_SINGLE, HIGH);
       DigiKeyboard.delay(50);
       digitalWrite(PIN_SINGLE, LOW);
       DigiKeyboard.delay(50);
    }
    digitalWrite(PIN_SINGLE, HIGH);
  #endif
}
