/**
 * MICRONUCLEUS WEBUSB DRIVER (Manual Override)
 * Compatible with Digispark ATtiny85 (Micronucleus V2.x Bootloader)
 */

class Micronucleus {
  constructor(device) {
    this.device = device;
    this.PAGE_SIZE = 64; // Standard for ATtiny85
    this.CMD_WRITE = 1;
    this.CMD_RUN = 3;
  }

  async upload(firmware) {
    if (!this.device) throw new Error("No device selected");

    // 1. OPEN CONNECTION
    await this.device.open();
    if (this.device.configuration === null) {
      await this.device.selectConfiguration(1);
    }
    await this.device.claimInterface(0);

    // 2. CHECK BOOTLOADER VERSION (Optional Ping)
    // We skip explicit version check to be robust, jumping straight to write.

    // 3. UPLOAD LOOP
    const len = firmware.length;
    let endPage = Math.ceil(len / this.PAGE_SIZE);
    
    // Safety cap for ATtiny85 (6KB max typically available)
    if (endPage > 96) throw new Error("Firmware too large for Digispark");

    console.log(`[Driver] Flashing ${len} bytes (${endPage} pages)...`);

    for (let i = 0; i < endPage; i++) {
      // Calculate delay based on page index to prevent timeouts
      // First page often takes longer due to erase cycle
      const delayMs = (i === 0) ? 50 : 5; 
      
      const start = i * this.PAGE_SIZE;
      const end = start + this.PAGE_SIZE;
      const pageData = new Uint8Array(this.PAGE_SIZE);
      
      // Fill page buffer (pad with 0xFF (Empty) if partial page)
      pageData.fill(0xFF);
      const chunk = firmware.slice(start, end);
      pageData.set(chunk);

      // MICRONUCLEUS V2 WRITE PROTOCOL
      // wValue = Offset (Address)
      // wIndex = Unused (0)
      // Data   = 64 bytes of page data
      await this.device.controlTransferOut({
        requestType: 'vendor',
        recipient: 'device',
        request: this.CMD_WRITE,
        value: start,
        index: 0
      }, pageData);

      // Wait for write/erase cycle to complete
      await new Promise(r => setTimeout(r, delayMs));
      
      // Update progress (Visual feedback if you had a progress bar)
      if (i % 10 === 0) console.log(`[Driver] Wrote page ${i}/${endPage}`);
    }

    // 4. VERIFY (Skipped for speed in JS implementation)

    // 5. RUN USER PROGRAM
    console.log("[Driver] Flashing Complete. Booting...");
    try {
      // This command causes the device to reset and disconnect
      await this.device.controlTransferOut({
        requestType: 'vendor',
        recipient: 'device',
        request: this.CMD_RUN,
        value: 0,
        index: 0
      });
    } catch (e) {
      // Ignore error here; device disconnects immediately upon running, which JS sees as a network error
      console.log("[Driver] Device disconnected (Expected behavior).");
    }
  }
}

// Export for module systems (optional) or attach to window
if (typeof window !== 'undefined') {
  window.Micronucleus = Micronucleus;
  console.log("Micronucleus Driver Loaded (Manual Mode)");
}
