# System Architecture

## Overview

BadUSB-Roulette is a system designed to flash multiple DuckyScript payloads onto a DigiSpark ATTiny85 device. It consists of a web-based frontend for script compilation and flashing, and a custom firmware that acts as a Virtual Machine (VM) to execute the payloads.

## Components

### 1. Web Frontend (`index.html`)

The frontend is a single-page application (SPA) that handles:

*   **Script Editing:** Users can write DuckyScript for three "chambers" (payload slots).
*   **Compilation:** The `compileDucky` function translates human-readable DuckyScript into a custom bytecode format optimized for the ATTiny85.
*   **Hex Patching:** The `patchHex` function takes a pre-compiled Intel Hex firmware binary (`roulette_single.hex` or `roulette_dual.hex`) and injects the compiled bytecode into a specific memory region (`PAYLOAD_STORAGE`).
*   **WebUSB Flashing:** It uses `micronucleus.js` to communicate with the DigiSpark's Micronucleus bootloader and flash the patched firmware directly from the browser.
*   **Escape Pod:** A fallback mechanism that generates a standalone `.ino` Arduino sketch (`BadUSB_Roulette_Recovery.ino`) with the payloads embedded as a C array. This allows users to manually compile and upload the firmware if WebUSB fails.

### 2. Firmware (`TheRevolver.ino`)

The firmware runs on the ATTiny85 and implements a lightweight VM:

*   **Virtual Machine:** It interprets the custom bytecode (Opcodes: END, DELAY, KEY, PRINT, PRINTLN).
*   **Payload Storage:** A 1KB region in Flash memory (`PROGMEM`) reserved for storing the three payloads.
*   **State Management:** It uses EEPROM to track the current "chamber" index. Each time the device boots, it increments the index to cycle through the payloads.
*   **Russian Roulette Logic:**
    *   It skips empty chambers.
    *   If plugged in, it waits for a "SAFE_WINDOW" (3 seconds). If still plugged in, it "fires" (executes the payload).
    *   If unplugged during the safe window, the state remains advanced, effectively selecting the next chamber for the next boot.

### 3. Firmware Configuration (`roulette_cfg.h`)

A header file to configure hardware specifics:
*   **LED Mode:** Supports single LED (Model A/B) or dual LED (custom hardware).
*   **Pin Mappings:** Defines which pins drive the LEDs.
*   **Game Physics:** Defines the number of chambers and the safe window duration.

## Data Flow

1.  **User Input:** DuckyScript -> `compileDucky()` -> Bytecode (Uint8Array).
2.  **Patching:** Base Firmware (.hex) + Bytecode -> `patchHex()` -> Patched Firmware (Uint8Array).
3.  **Flashing:** Patched Firmware -> `Micronucleus.upload()` -> ATTiny85 Flash Memory.
4.  **Execution:** ATTiny85 Boot -> Read EEPROM -> Select Chamber -> Read Bytecode from Flash -> Execute via VM.
