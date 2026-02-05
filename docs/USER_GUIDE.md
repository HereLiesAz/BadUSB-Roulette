# User Guide

## Prerequisites

### Hardware
*   **Digispark ATTiny85** (Revision A or B).
*   **Computer** with a USB port.
*   **Supported Browser:** Chrome, Edge, or any browser that supports WebUSB.

### Drivers (Windows Only)
The standard Digispark driver (`libusb-win32`) is incompatible with WebUSB. You **must** install the `WinUSB` driver.

1.  Download and run [Zadig](https://zadig.akeo.ie/).
2.  Plug in your Digispark.
3.  In Zadig, go to **Options** -> **List All Devices**.
4.  Select **Digispark Bootloader** from the dropdown.
    *   *Note: If it doesn't appear, unplug and replug the device while Zadig is open.*
5.  In the driver selection box (right side), choose **WinUSB**.
6.  Click **Replace Driver** (or Install Driver).

### Permissions (Linux Only)
Linux requires specific udev rules to allow non-root access to the USB device.

1.  Open a terminal.
2.  Run the following command to create a udev rule:
    ```bash
    echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="16d0", ATTR{idProduct}=="0753", MODE="0666", GROUP="plugdev"' | sudo tee /etc/udev/rules.d/99-digispark.rules
    ```
3.  Reload the udev rules:
    ```bash
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    ```
4.  Unplug and replug your device.

## Flashing the Device

1.  Open `index.html` in a supported browser.
2.  **Select Target Model:**
    *   **Universal Single:** For standard Digisparks (uses Pin 0 and Pin 1 for LEDs).
    *   **Dual LED:** For custom boards with Green (P0) and Red (P1) LEDs.
3.  **Enter Payloads:**
    *   You have three "chambers" (C1, C2, C3).
    *   Enter your DuckyScript code into the text areas.
    *   You can use the "MUTATE" dropdowns to load example payloads.
4.  **Connect Device:** Plug in your Digispark.
5.  **Flash:** Click the **FLASH FIRMWARE** button.
    *   Accept the browser's permission prompt to connect to the "Digispark Bootloader".
    *   Wait for the progress log to say "/// DONE ///".

## Using the Device

The BadUSB-Roulette operates like a game of Russian Roulette. It cycles through the loaded payloads each time it is plugged in.

1.  **Plug it in.**
2.  **Watch the LED:**
    *   **Blinking:** The device is identifying which chamber is currently selected (1 blink = Chamber 1, 2 blinks = Chamber 2, etc.).
    *   **Solid/Armed:** After identification, the LED will turn solid (or Red in Dual Mode). This is the "Safe Window" (3 seconds).
3.  **Make a Choice:**
    *   **Unplug Immediately:** To skip this chamber and select the next one for the next time.
    *   **Wait:** If you leave it plugged in past the Safe Window, the device will "Fire" and execute the payload.

## Troubleshooting

*   **"Driver not loaded":** Ensure `micronucleus.js` is in the `firmware/` folder and that your browser supports WebUSB.
*   **"Access Denied" (Linux):** Check your udev rules.
*   **Device not found:** The Digispark bootloader is only active for 5 seconds after plugging in. You must click "Flash" and select the device quickly, or plug it in *after* clicking Flash when prompted.

## The "Escape Pod"

If WebUSB fails or you cannot install drivers, use the **GENERATE .INO** button.

1.  Click **GENERATE .INO**.
2.  This will download a file named `BadUSB_Roulette_Recovery.ino`.
3.  Open this file in the Arduino IDE.
4.  Install the **Digistump AVR Boards** package in Arduino IDE.
5.  Select **Digispark (Default - 16.5mhz)** as the board.
6.  Click **Upload** to flash the device manually.
