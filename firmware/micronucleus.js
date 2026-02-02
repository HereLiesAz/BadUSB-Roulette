/*
  Micronucleus WebUSB Driver (Robust Timing Edition)
  Target: ATtiny85 (Digispark) running Micronucleus V2.x Bootloader
*/

(function() {
  class Micronucleus {
    constructor(device) {
      this.device = device;
      this.PAGE_SIZE = 64; 
      this.CMD_WRITE = 1;
      this.CMD_RUN = 3; 
    }

    async open() {
      if (!this.device.opened) {
        await this.device.open();
      }
      if (this.device.configuration === null) {
        await this.device.selectConfiguration(1);
      }
      // Micronucleus uses Interface 0
      try {
        await this.device.claimInterface(0);
      } catch(e) {
        console.warn("Interface already claimed or unavailable (ignoring).");
      }
    }

    async upload(firmwareData) {
      await this.open();

      const len = firmwareData.length;
      const totalPages = Math.ceil(len / this.PAGE_SIZE);
      
      // Safety check for ATtiny85 (approx 6KB usable)
      if (totalPages > 96) throw new Error("Firmware too large for device memory.");

      console.log(`[Micronucleus] Flashing ${len} bytes (${totalPages} pages)...`);

      // 1. ERASE & WRITE LOOP
      for (let i = 0; i < totalPages; i++) {
        // Prepare 64-byte page buffer
        const pageBuffer = new Uint8Array(this.PAGE_SIZE).fill(0xFF);
        const start = i * this.PAGE_SIZE;
        const end = start + this.PAGE_SIZE;
        const chunk = firmwareData.slice(start, end);
        pageBuffer.set(chunk);

        // TIMING IS CRITICAL HERE
        // The first page write triggers a block erase which takes time.
        // Standard Write: ~4-5ms
        // Erase + Write: ~50-100ms
        // We use conservative values to prevent "Transfer Error"
        const delayMs = (i === 0) ? 250 : 20; 

        // SEND: Control Transfer to Device
        // request: 1 (Write)
        // value: Start Address
        // index: 0
        await this.device.controlTransferOut({
          requestType: 'vendor',
          recipient: 'device',
          request: this.CMD_WRITE,
          value: start,
          index: 0
        }, pageBuffer);

        // SLEEP: Give the tiny CPU time to write to flash
        await new Promise(resolve => setTimeout(resolve, delayMs));
        
        // Optional: Log progress every 10 pages
        if (i % 10 === 0) console.log(`[Micronucleus] Wrote page ${i+1}/${totalPages}`);
      }

      console.log("[Micronucleus] Write Complete. Resetting...");
      
      // 2. RESET / RUN
      try {
        await this.device.controlTransferOut({
          requestType: 'vendor',
          recipient: 'device',
          request: this.CMD_RUN,
          value: 0,
          index: 0
        });
      } catch (e) {
        // The device resets immediately, often causing a 'NetworkError' or disconnect in JS.
        // This is actually success.
        console.log("[Micronucleus] Device reset (Success).");
      }
    }
  }

  // Expose to Global Window
  if (typeof window !== 'undefined') {
    window.Micronucleus = Micronucleus;
    console.log("Micronucleus Driver Loaded (Robust Timing).");
  }
})();
