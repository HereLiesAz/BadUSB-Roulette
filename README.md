# BadUSB-Roulette (Web-Flasher Edition)

The BadUSB is a revolver. The barrel holds three bullets.
Plug the BadUSB in to see which bullet is in the chamber.
Unplug it to select the next bullet.
Or leave it in to fire that bullet.

Just like Russian Roulette, except you control the bullet.
Which, I guess, makes it normal Roulette... the kind we all necessarily play when a revolver is involved.

## How It Actually Works
This is **NOT** a drag-and-drop folder system. That was a lie.
This is a **Web-Compiler**.

1. **The Repo:** Hosts a website (`index.html`) and two compiled firmware templates.
2. **The Browser:** You load DuckyScripts into the website.
3. **The Flash:** The website compiles your scripts to bytecode, patches the firmware binary in memory, and flashes it to your DigiSpark via WebUSB.

## Instructions

### Phase 1: Deployment
1. **Fork** this repository.
2. Go to **Settings > Pages**.
3. Set the Source to **GitHub Actions** (or `gh-pages` branch if the Action has already run).
4. Wait for the `Deploy Armory` action to finish.

### Phase 2: Ammunition
1. Visit your deployed site: `https://YOUR_USER.github.io/BadUSB-Roulette/`
2. **Connect** your DigiSpark ATTiny85.
3. **Write** or **Load** DuckyScript into the three chambers on the screen.
   - Chamber 1: Windows Payload?
   - Chamber 2: Mac Payload?
   - Chamber 3: Linux Payload?
4. **Click "FLASH FIRMWARE"**.
5. The site will compile the scripts, inject them into the hex file, and burn it to the device.

### Phase 3: Violence
1. Plug the device into a target.
2. It blinks to tell you which chamber is active (1, 2, or 3).
3. **Wait 3 seconds** to fire.
4. **Unplug immediately** to cycle to the next chamber without firing.
5. Empty chambers are automatically skipped.

## Payload Syntax (DuckyScript)
- `STRING hello world` - Types text.
- `DELAY 500` - Waits 500ms.
- `GUI r` - Windows Run / Command Space.
- `ENTER` - Presses Enter.
