# BadUSB-Roulette for the DigiSpark ATTiny85. 

  The BadUSB is a revolver. <br>
    The barrel holds three bullets. <br>
      Plug the BadUSB in to see which bullet is in the chamber. <br>
        Unplug it to select the next bullet. <br>
          Or leave it in to fire that bullet. <br><br>
          
   Just like Russian Roulette, except you control the bullet. <br>
    Which, I guess, makes it normal Roulette... the kind we all necessarily play when a revolver is involved. <br>
    <br>  And this repository is a munitions factory, designed for "Drop-and-Load" simplicity. <br>
    <linebreak>
      You do not need to know C++. You do not need to convert your duckyscript. You do not need to install Arduino. <br>
    Just load that gun of yours and cock it. <br><br>
    
                  BasUSB-Roulette/
    Don't touch.  ├── .github/workflows/
    Don't touch.  │   └── factory.yml        <-- THE ENGINE. This GitHub Action watches for your
                  │                              changes, converts your scripts, compiles the
                  │                              firmware, and deploys the Flasher Website.
                  │
    Don't touch.  ├── src/
                  │   ├── chamber-one/       <-- WINDOWS PAYLOADS, for example. Drop .txt or .ino files here.
    Delete.       │   │   ├── 00_setup.txt
    Replace.      │   │   └── 01_exploit.txt
                  │   │
                  │   ├── chamber-two/       <-- MAC PAYLOADS, for example. Drop files here.
    Replace.      │   │   └── payload.txt
                  │   │
                  │   ├── chamber-three/     <-- LINUX/MOBILE PAYLOADS, for example. Drop files here.
    Replace.      │   │   └── payload.txt
                  │
    Don't touch.  ├── scripts/
                  │   └── weaponizer.py      <-- THE TRANSLATOR. A Python script that converts
                  │                              DuckyScript to C++ and merges multiple files
                  │                              into a single payload header.
                  │
    Don't touch.  ├── config.h               <-- HARDWARE SETTINGS. Defines pinouts and timings.
                  │
    Don't touch.  └── Roulette.ino           <-- THE TRIGGER. The main logic that handles
                                                 entropy, LED signaling, and payload execution.

<br><br>
## Operational Instructions
 
  ### Phase 1: The Setup
  1. Fork this Repository to your own GitHub account.
  
  2. Enable GitHub Actions in the "Actions" tab.
  
  3. Go to Settings > Pages and set the Source to gh-pages (root).
  
  ### Phase 2: The Loadout
  1. Navigate to src/chamber-one/ (or two. Or three.)
  
  2. Delete the placeholder files.
  
  3. Upload your payloads:
  
   - Duckyscript (.txt): Standard syntax (GUI r, STRING hello, DELAY 500).
  
   - Arduino C++ (.ino): Raw code for advanced, raw users.
  
  4. Note that files are executed in alphabetical order.
  
  5. Commit changes.
  
  ### Phase 3: The Fabrication
  1. Wait ~60 seconds.
  
  2. The Factory Action is running. It will:
  
   - Convert your Duckyscript to C++.
  
   - Compile two firmware versions (Single LED & Dual LED).
  
   - Deploy a Web Flasher to your repository's website.
  
  ### Phase 4: The Flash
  1. Visit your deployed site: WHATS_YOUR_FACE.github.io/BadUSB-Roulette/
  
  2. Click "ARM DEVICE."
  
  3. Plug in your DigiSpark ATTiny85 when prompted.
  
  4. Pew pew.

