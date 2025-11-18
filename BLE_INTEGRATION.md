# BLE Integration Guide

## Overview

The BLE handler provides wireless button control for the AquariumPet via Bluetooth Low Energy using NimBLE on ESP32.

## Files Created

- `src/ble_handler.h` - BLE API header
- `src/ble_handler.cpp` - BLE implementation using NimBLE
- Updated `src/Buttons.h` - Added `setBleMask()` method
- Updated `src/Buttons.cpp` - BLE button integration
- Updated `platformio.ini` - Added NimBLE-Arduino library dependency

## Integration in main.cpp

Add to your includes (ESP32 only):
```cpp
#ifdef ESP32
#include "ble_handler.h"
#endif
```

Add to `setup()` function after `initDisplay()`:
```cpp
#ifdef ESP32
    initBLE();
#endif
```

Optionally add to `loop()` if needed for housekeeping:
```cpp
#ifdef ESP32
    updateBLE();
#endif
```

## BLE Protocol

### Service & Characteristic
- **Device Name**: `AquariumPet`
- **Service UUID**: `8B3D0001-57B4-4DFE-8A3E-2F0D5A5B8C01`
- **Characteristic UUID**: `8B3D0002-57B4-4DFE-8A3E-2F0D5A5B8C01`
- **Properties**: WRITE_NR (Write Without Response), NOTIFY

### Button Payload Format

**Simple ASCII Mode (recommended for testing):**
Send a single ASCII character:
- `'l'` or `'L'` - LEFT button
- `'o'` or `'O'` - OK button
- `'r'` or `'R'` - RIGHT button

**Bitmask Mode (for multiple buttons):**
Send a single byte (uint8_t):
- **Bit 0** (0x01): LEFT button
- **Bit 1** (0x02): OK button
- **Bit 2** (0x04): RIGHT button

Examples:
- `'l'` or `0x01` - LEFT pressed
- `'o'` or `0x02` - OK pressed
- `'r'` or `0x04` - RIGHT pressed
- `0x03` - LEFT + OK pressed (bitmask only)
- `0x00` - All buttons released (automatically sent on disconnect)

## Client Example (Python with Bleak)

```python
import asyncio
from bleak import BleakClient, BleakScanner

DEVICE_NAME = "AquariumPet"
SERVICE_UUID = "8B3D0001-57B4-4DFE-8A3E-2F0D5A5B8C01"
CHAR_UUID = "8B3D0002-57B4-4DFE-8A3E-2F0D5A5B8C01"

async def send_button(client, mask):
    await client.write_gatt_char(CHAR_UUID, bytes([mask]), response=False)

async def main():
    device = await BleakScanner.find_device_by_name(DEVICE_NAME)
    if not device:
        print("Device not found")
        return
    
    async with BleakClient(device) as client:
        # Press LEFT button
        await send_button(client, 0x01)
        await asyncio.sleep(0.1)
        await send_button(client, 0x00)  # Release
        
        # Press OK button
        await send_button(client, 0x02)
        await asyncio.sleep(0.1)
        await send_button(client, 0x00)

asyncio.run(main())
```

## Client Example (Web Bluetooth API)

```javascript
const SERVICE_UUID = '8b3d0001-57b4-4dfe-8a3e-2f0d5a5b8c01';
const CHAR_UUID = '8b3d0002-57b4-4dfe-8a3e-2f0d5a5b8c01';

async function connectAquarium() {
    const device = await navigator.bluetooth.requestDevice({
        filters: [{ name: 'AquariumPet' }],
        optionalServices: [SERVICE_UUID]
    });
    
    const server = await device.gatt.connect();
    const service = await server.getPrimaryService(SERVICE_UUID);
    const char = await service.getCharacteristic(CHAR_UUID);
    
    return char;
}

async function pressButton(char, button) {
    // button: 'left' | 'ok' | 'right'
    const masks = { left: 0x01, ok: 0x02, right: 0x04 };
    await char.writeValueWithoutResponse(new Uint8Array([masks[button]]));
    await new Promise(resolve => setTimeout(resolve, 100));
    await char.writeValueWithoutResponse(new Uint8Array([0x00]));
}

// Usage
const char = await connectAquarium();
await pressButton(char, 'left');
await pressButton(char, 'ok');
```

## Debug Output

When `DEBUG_BUTTONS` is defined in `config.h`, the following messages are logged:

```
[BLE] Initializing BLE server...
[BLE] BLE server started and advertising
[BLE] Device name: AquariumPet
[BLE] Service UUID: 8B3D0001-57B4-4DFE-8A3E-2F0D5A5B8C01
[BLE] Characteristic UUID: 8B3D0002-57B4-4DFE-8A3E-2F0D5A5B8C01
[BLE] Client connected
[BLE] Received button mask: 0x01 (LEFT=1, OK=0, RIGHT=0)
[BLE] Button mask updated: 0x01
[BLE] Client disconnected - cleared button mask
[BLE] Restarted advertising
```

## Safety Features

1. **Automatic cleanup on disconnect**: When a BLE client disconnects, the button mask is automatically cleared to prevent stuck buttons.

2. **No SPI/Display operations in callbacks**: All BLE callbacks are minimal and only call `Buttons.setBleMask()`, which is safe to call from BLE context.

3. **ESP32-only**: All BLE code is wrapped in `#ifdef ESP32` to ensure it doesn't interfere with Teensy builds.

## Notes

- BLE buttons are merged with physical buttons - both sources work simultaneously
- Button presses from BLE trigger the same debouncing logic as physical buttons
- The BLE characteristic supports NOTIFY for future two-way communication (not currently used)
- Advertising automatically restarts after client disconnect
