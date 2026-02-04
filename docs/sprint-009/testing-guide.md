# Sprint #009 Testing Guide

## What to Test

Verify that device names (not MAC addresses) are displayed in the SharePlugin device selection list for both paired and newly discovered devices.

## Test Scenarios

### Test 1: Already Paired Devices
**Expected:** Device names should appear immediately

1. Open any app with share functionality (e.g., Gallery)
2. Select a file to share
3. Choose "Bluetooth" from share options
4. Verify that already paired devices show their friendly names
   - Example: "John's Phone" instead of "AA:BB:CC:DD:EE:FF"

### Test 2: Newly Discovered Devices (Main Fix)
**Expected:** MAC address shown briefly, then updated to device name

1. Ensure you have a discoverable Bluetooth device nearby that is NOT paired
2. Open SharePlugin via ContentHub
3. Wait for device discovery to complete
4. **Initially:** May show MAC address briefly
5. **After 1-2 seconds:** Should update to show device name
6. Monitor console output for debug messages:
   ```
   New device: AA:BB:CC:DD:EE:FF
   Resolved device name: AA:BB:CC:DD:EE:FF for address: AA:BB:CC:DD:EE:FF
   Device name updated: AA:BB:CC:DD:EE:FF -> Samsung Galaxy S10
   ```

### Test 3: File Transfer Still Works
**Expected:** File transfer functionality unchanged

1. Select a device from the list (by name)
2. Share a file
3. Verify transfer completes successfully
4. Confirm the correct device received the file

## Monitoring Logs

To see detailed debug output:

```bash
# View SharePlugin logs
clickable logs --filter shareplugin

# Or system logs
journalctl -f | grep -i bluetooth
```

## What to Look For

### Success Indicators:
- ✅ Paired devices show names immediately
- ✅ New devices show MAC initially, then update to name within 1-2 seconds
- ✅ Console shows "Device name updated:" messages
- ✅ File transfer works correctly
- ✅ No crashes or errors

### Failure Indicators:
- ❌ New devices permanently show MAC addresses
- ❌ No "Device name updated:" messages in logs
- ❌ D-Bus errors in system logs
- ❌ Application crashes when discovering devices

## Known Behavior

- **Initial MAC Display:** For newly discovered devices, the MAC address may appear for 1-2 seconds before being replaced with the device name. This is normal and expected due to BlueZ's asynchronous property population.

- **Unnamed Devices:** Some Bluetooth devices don't broadcast names during discovery. These will continue to show MAC addresses (fallback behavior).

- **D-Bus Timing:** The name update depends on when BlueZ populates device properties. Network conditions and device response times can affect this (typically 500ms-2s).

## Troubleshooting

### If names don't update:
1. Check system Bluetooth is working: `bluetoothctl power on`
2. Verify BlueZ service is running: `systemctl status bluetooth`
3. Check D-Bus permissions in AppArmor (should already be configured)
4. Look for D-Bus errors in logs

### If monitoring isn't working:
- Verify the device path construction is correct
- Check that `InterfacesAdded` and `PropertiesChanged` signals are being received
- Enable D-Bus monitoring: `dbus-monitor --system "interface='org.bluez.Device1'"`
