# Bytecode Specification

The BadUSB-Roulette uses a custom bytecode format to execute DuckyScript payloads efficiently on the ATTiny85. The bytecode is stored in a 1KB region of Flash memory (`PAYLOAD_STORAGE`).

## Memory Layout

The `PAYLOAD_STORAGE` array (1024 bytes) is structured as follows:

| Offset | Size | Description |
| :--- | :--- | :--- |
| `0x00` | 4 Bytes | **Magic Header:** `0xCA, 0xFE, 0xBA, 0xBE` |
| `0x04` | 2 Bytes | **Chamber 1 Pointer:** Offset to start of Payload 1 (Big Endian) |
| `0x06` | 2 Bytes | **Chamber 2 Pointer:** Offset to start of Payload 2 (Big Endian) |
| `0x08` | 2 Bytes | **Chamber 3 Pointer:** Offset to start of Payload 3 (Big Endian) |
| `0x0A` | ... | **Payload Data:** Variable length bytecode sequences. |

*Note: Pointers are relative to the start of `PAYLOAD_STORAGE`. A pointer value of `0x0000` indicates an empty chamber.*

## Opcodes

The Virtual Machine (VM) interprets the following opcodes. All multi-byte integers are Big Endian.

### `0x00` - OP_END
Terminates the execution of the current payload.

### `0x01` - OP_DELAY
Pauses execution for a specified duration.
*   **Arg 1 (1 Byte):** High Byte of milliseconds.
*   **Arg 2 (1 Byte):** Low Byte of milliseconds.
*   **Total Size:** 3 Bytes.

### `0x02` - OP_KEY
Sends a keystroke with optional modifiers.
*   **Arg 1 (1 Byte):** Modifier Bitmask (e.g., Shift, Ctrl).
    *   `CTRL`: 0x01
    *   `SHIFT`: 0x02
    *   `ALT`: 0x04
    *   `GUI`: 0x08
*   **Arg 2 (1 Byte):** HID Usage ID for the key.
*   **Total Size:** 3 Bytes.

### `0x03` - OP_PRINT
Types a string of characters.
*   **Arg 1 (1 Byte):** Length of the string (`L`).
*   **Args 2..L+1:** The ASCII characters to type.
*   **Total Size:** 2 + L Bytes.

### `0x04` - OP_PRINTLN
Types a string of characters followed by a newline (Enter key).
*   **Arg 1 (1 Byte):** Length of the string (`L`).
*   **Args 2..L+1:** The ASCII characters to type.
*   **Total Size:** 2 + L Bytes.

## Example

**DuckyScript:**
```
DELAY 500
STRING hello
```

**Bytecode:**
```
0x01, 0x01, 0xF4,       // DELAY 500ms (0x01F4)
0x03, 0x05, 'h', 'e', 'l', 'l', 'o', // PRINT "hello" (Length 5)
0x00                    // END
```
