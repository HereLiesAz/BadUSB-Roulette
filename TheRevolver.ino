/*
   PROJECT: BadUSB Revolver (Interpreter Edition)
   HARDWARE: ATtiny85 (Digispark)
   
   This version does not contain hardcoded payloads.
   It contains a Bytecode Interpreter and a reserved memory block.
   The Web Interface injects data into this block.
 */

#include <EEPROM.h>
#include "DigiKeyboard.h"
#include "config.h"

// ==========================================
//      VIRTUAL MACHINE DEFINITIONS
// ==========================================
#define OP_END      0x00
#define OP_DELAY    0x01 // [OP] [TIME_MS_H] [TIME_MS_L]
#define OP_KEY      0x02 // [OP] [MOD] [KEY]
#define OP_PRINT    0x03 // [OP] [LEN] [CHAR1] [CHAR2]...
#define OP_PRINTLN  0x04 // [OP] [LEN] [CHAR1]... (Adds Enter)

// RESERVE 2KB FOR PAYLOADS (Magic Header: CAFE BABE)
// The web flasher looks for this pattern to know where to write.
const uint8_t PAYLOAD_STORAGE[2048] PROGMEM = {
  0xCA, 0xFE, 0xBA, 0xBE, 
  0x00, 0x00, 0x00, 0x00  // Padding/Offsets (Managed by JS)
};

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
    pinMode(PIN_SINGLE, OUTPUT);
    digitalWrite(PIN_SINGLE, LOW);
  #endif

  // 2. ROTATION LOGIC (Entropy)
  byte mode = EEPROM.read(0);
  if (mode >= TOTAL_CHAMBERS) mode = 0;
  
  byte nextMode = (mode + 1) % TOTAL_CHAMBERS;
  EEPROM.write(0, nextMode);

  // 3. IDENTIFICATION PHASE
  DigiKeyboard.delay(1000); 

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
  // The JS compiler generates a specific offset for each chamber.
  // We assume the header structure is:
  // [CA FE BA BE] [OFFSET_1_H] [OFFSET_1_L] [OFFSET_2_H] [OFFSET_2_L] [OFFSET_3_H] [OFFSET_3_L]
  
  uint16_t offset_loc = 4 + (mode * 2);
  uint8_t off_h = pgm_read_byte(&PAYLOAD_STORAGE[offset_loc]);
  uint8_t off_l = pgm_read_byte(&PAYLOAD_STORAGE[offset_loc + 1]);
  uint16_t payload_addr = (off_h << 8) | off_l;

  // Safety check: If offset is 0 or out of bounds (still FF), do nothing.
  if (payload_addr > 0 && payload_addr < 2048) {
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
    
    if (opcode == OP_END) {
      break; 
    }
    
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
    
    // Safety break for runaways
    if (ptr >= 2048) break;
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
