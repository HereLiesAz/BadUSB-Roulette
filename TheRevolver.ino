name: Deploy System (Unified)

on:
  push:
    branches: [ "main", "master" ]
    paths-ignore:
      - 'firmware/**' # CRITICAL: Prevent infinite build loops
  workflow_dispatch:

permissions:
  contents: write

jobs:
  build-and-deploy:
    runs-on: ubuntu-latest
    steps:
      - name: Checkout Source
        uses: actions/checkout@v3
        with:
          ref: ${{ github.head_ref }}

      # --- SETUP ---
      - name: Setup Arduino CLI
        uses: arduino/setup-arduino-cli@v1

      - name: Install Digispark Core
        run: |
          arduino-cli config init
          arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/digistump/arduino-boards-index/master/package_digistump_index.json
          arduino-cli core update-index
          arduino-cli core install digistump:avr

      # --- BUILD ---
      - name: Compile Firmware
        run: |
          # 1. Clean Inventory
          mkdir -p firmware
          rm -f firmware/*.hex
          echo "Inventory Cleared."

          # 2. Compile SINGLE LED
          echo ">>> COMPILING SINGLE LED..."
          mkdir -p build/single_env/Sketch
          cp TheRevolver.ino build/single_env/Sketch/Sketch.ino
          cp roulette_cfg.h build/single_env/Sketch/roulette_cfg.h
          
          # Patch Config: Set DUAL_LED_MODE to 0
          sed -i 's/#define DUAL_LED_MODE 1/#define DUAL_LED_MODE 0/' build/single_env/Sketch/roulette_cfg.h
          
          # Compile
          arduino-cli compile --fqbn digistump:avr:digispark-tiny --output-dir ./build/single build/single_env/Sketch
          
          # Extract
          if [ -f "./build/single/Sketch.ino.hex" ]; then
            cp "./build/single/Sketch.ino.hex" "./firmware/roulette_single.hex"
          else
            echo "CRITICAL: Single Hex Failed to Build"
            exit 1
          fi

          # 3. Compile DUAL LED
          echo ">>> COMPILING DUAL LED..."
          mkdir -p build/dual_env/Sketch
          cp TheRevolver.ino build/dual_env/Sketch/Sketch.ino
          cp roulette_cfg.h build/dual_env/Sketch/roulette_cfg.h
          
          # Patch Config: Set DUAL_LED_MODE to 1
          sed -i 's/#define DUAL_LED_MODE 0/#define DUAL_LED_MODE 1/' build/dual_env/Sketch/roulette_cfg.h
          
          # Compile
          arduino-cli compile --fqbn digistump:avr:digispark-tiny --output-dir ./build/dual build/dual_env/Sketch
          
          # Extract
          if [ -f "./build/dual/Sketch.ino.hex" ]; then
            cp "./build/dual/Sketch.ino.hex" "./firmware/roulette_dual.hex"
          else
            echo "CRITICAL: Dual Hex Failed to Build"
            exit 1
          fi

      # --- COMMIT BACK ---
      - name: Commit Firmware to Repo
        run: |
          git config --global user.name "GitHub Actions"
          git config --global user.email "actions@github.com"
          
          # Stage the new firmware files
          git add firmware/*.hex
          
          # Check for changes. If changed, commit and push.
          # [skip ci] is crucial to prevent the commit from triggering another build.
          git diff --quiet && git diff --staged --quiet || (git commit -m "Reload: Updated Firmware Artifacts [skip ci]" && git push)

      # --- DEPLOY SITE ---
      - name: Stage & Deploy Website
        run: |
          mkdir -p public
          cp index.html public/index.html
          # Copy the firmware folder (which now exists in the repo) to the deploy folder
          cp -r firmware public/firmware
          touch public/.nojekyll

      - name: Deploy to GitHub Pages
        uses: peaceiris/actions-gh-pages@v3
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: ./public
          commit_message: "Deploy: Unified Release"
