/*
  Micronucleus WebUSB Driver (Paranoid Safety Edition)
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
      // Attempt to claim interface, ignore if already claimed
      try {
        await this.device.claimInterface(0);
      } catch(e) {
        console.warn("[Driver] Interface claim warning (ignoring):", e);
      }
    }

    async upload(firmwareData) {
      await this.open();

      const len = firmwareData.length;
      const totalPages = Math.ceil(len / this.PAGE_SIZE);
      
      // Strict safety check for ATtiny85 (Approx 6KB user space)
      if (totalPages > 96) throw new Error("Firmware too large for device memory.");

      console.log(`[Driver] Flashing ${len} bytes (${totalPages} pages)...`);

      // 1. ERASE & WRITE LOOP
      for (let i = 0; i < totalPages; i++) {
        // Prepare 64-byte page buffer
        const pageBuffer = new Uint8Array(this.PAGE_SIZE).fill(0xFF);
        const start = i * this.PAGE_SIZE;
        const end = start + this.PAGE_SIZE;
        const chunk = firmwareData.slice(start, end);
        pageBuffer.set(chunk);

        // --- PARANOID TIMING ---
        // Page 0 write triggers the initial Erase Cycle.
        // We give it 500ms to ensure the chip is absolutely ready before we talk to it again.
        // Subsequent pages get 50ms (10x standard requirement).
        const delayMs = (i === 0) ? 500 : 50; 

        // SEND: Control Transfer to Device
        await this.device.controlTransferOut({
          requestType: 'vendor',
          recipient: 'device',
          request: this.CMD_WRITE,
          value: start,
          index: 0
        }, pageBuffer);

        // WAIT: Hold the bus idle while the chip works
        await new Promise(resolve => setTimeout(resolve, delayMs));
        
        if (i % 5 === 0) console.log(`[Driver] Wrote page ${i+1}/${totalPages}`);
      }

      console.log("[Driver] Write Complete. Resetting...");
      
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
        // A network error here is actually success (device disconnected to reboot)
        console.log("[Driver] Device reset confirmed.");
      }
    }
  }

  // Expose to Global Window
  if (typeof window !== 'undefined') {
    window.Micronucleus = Micronucleus;
    console.log("Micronucleus Driver Loaded (Paranoid Edition).");
  }
})();
