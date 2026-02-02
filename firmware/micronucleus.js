/*
  Micronucleus WebUSB Driver
  Target: ATtiny85 (Digispark) running Micronucleus V2.x Bootloader
*/

(function() {
  class Micronucleus {
    constructor(device) {
      this.device = device;
      this.PAGE_SIZE = 64; 
      this.CMD_WRITE = 1;
      this.CMD_RUN = 3; // Nuke/Run command
    }

    async open() {
      await this.device.open();
      if (this.device.configuration === null) {
        await this.device.selectConfiguration(1);
      }
      // Micronucleus typically uses Interface 0
      await this.device.claimInterface(0);
    }

    async upload(firmwareData) {
      if (!this.device.opened) await this.open();

      const len = firmwareData.length;
      // Calculate total pages (64 bytes per page)
      const totalPages = Math.ceil(len / this.PAGE_SIZE);
      
      // Safety check for ATtiny85 (approx 6KB usable)
      if (totalPages > 96) throw new Error("Firmware too large for device memory.");

      console.log(`[Micronucleus] Flashing ${len} bytes (${totalPages} pages)...`);

      for (let i = 0; i < totalPages; i++) {
        // Prepare 64-byte page buffer
        const pageBuffer = new Uint8Array(this.PAGE_SIZE).fill(0xFF);
        const start = i * this.PAGE_SIZE;
        const end = start + this.PAGE_SIZE;
        const chunk = firmwareData.slice(start, end);
        pageBuffer.set(chunk);

        // DELAY: First page (erase cycle) needs more time. Subsequent pages are faster.
        const delayMs = (i === 0) ? 50 : 5; 

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

        // WAIT: Allow the tiny CPU to write to flash memory
        await new Promise(resolve => setTimeout(resolve, delayMs));
      }

      console.log("[Micronucleus] Write Complete.");
      
      // RESET / RUN
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
    console.log("Micronucleus Driver Loaded (Hardcoded).");
  }
})();
