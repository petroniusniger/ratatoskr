# Sprint #002: Bluetooth Service Layer - Technical Documentation

**Date**: 2025-12-19  
**Version**: 0.0.1  
**Status**: Completed

## Overview

This sprint implemented the core Bluetooth service layer for the Ratatoskr application, providing device discovery, connection management, and OBEX file transfer capabilities.

## Architecture

### Service Layer Components

1. **BluetoothManager** (`bluetoothmanager.h/cpp`)
   - Central Bluetooth adapter management
   - D-Bus interface: `org.bluez.Adapter1`
   - Properties: powered, discoverable, adapter address/name
   - Methods: initialize(), startDiscovery(), stopDiscovery()

2. **DeviceDiscovery** (`devicediscovery.h/cpp`)
   - Device discovery and tracking
   - QAbstractListModel for QML integration
   - D-Bus interfaces: `org.bluez.Device1`, `org.freedesktop.DBus.ObjectManager`
   - Monitors InterfacesAdded/InterfacesRemoved signals
   - BluetoothDevice class represents individual devices

3. **OBEXTransferService** (`obextransfer.h/cpp`)
   - Outgoing file transfer via OBEX
   - D-Bus interfaces: `org.bluez.obex.Client1`, `org.bluez.obex.ObjectPush1`
   - OBEXTransfer class tracks individual transfers
   - Session management for OBEX connections

### D-Bus Communication

#### System Bus (org.bluez)
- Bluetooth adapter management
- Device discovery and pairing
- Connection status monitoring

#### Session Bus (org.bluez.obex)
- OBEX client operations
- File transfer sessions
- Transfer progress tracking

### UI Components

#### Main Page
- Displays received file transfers
- Action to navigate to Send File page
- Transfer list with status indicators

#### Send File Page
- Device discovery interface
- Scan/stop scan functionality
- Device list with pairing/connection status
- Bluetooth status indicators

## Code Changes

### New Files
- `ratatoskr/bluetoothmanager.h` - BluetoothManager header
- `ratatoskr/bluetoothmanager.cpp` - BluetoothManager implementation
- `ratatoskr/devicediscovery.h` - DeviceDiscovery header
- `ratatoskr/devicediscovery.cpp` - DeviceDiscovery implementation
- `ratatoskr/obextransfer.h` - OBEXTransferService header
- `ratatoskr/obextransfer.cpp` - OBEXTransferService implementation

### Modified Files
- `ratatoskr/Main.qml` - Updated UI with device discovery page
- `ratatoskr/main.cpp` - Registered new services and types
- `ratatoskr/CMakeLists.txt` - Added new source files
- `ratatoskr/ratatoskr.apparmor` - Fixed AppArmor policy (removed unconfined)
- `ratatoskr/shareplugin/shareplugin.apparmor` - Fixed AppArmor policy (removed unconfined)

### Deprecated Component Fixes
- Replaced `UbuntuShape` with `Image` and `Rectangle`
- Fixed rendering issue in emulator

### AppArmor Security Policy
- Removed `unconfined` template (not allowed for production)
- Added `bluetooth` policy group for BlueZ/OBEX access
- Implements ADR-002 security decision
- Status: Ready for manual review (standard for BT apps)

## Build Status

✅ **Build**: Successful  
✅ **Compilation**: No errors  
✅ **AppArmor Policy**: Fixed - using 'bluetooth' policy group (requires manual review)

**AppArmor Status**: Changed from automatic rejection (`unconfined` not allowed) to manual review (`bluetooth` is reserved for vetted apps). This is **expected and correct** for Bluetooth applications.

Build command: `clickable build`

## Testing Status

### Emulator Testing
- ✅ Application launches successfully
- ✅ Main window renders correctly (UbuntuShape fix resolved empty window)
- ⚠️ Bluetooth services unavailable in emulator (expected)
- ⚠️ D-Bus system bus may not be fully available

### Hardware Testing Required
- Device discovery functionality
- Bluetooth pairing
- File transfer operations
- Connection state management

**Note**: Hardware testing pending due to ADB connectivity issues in VM environment.

## API Reference

### QML Context Properties

```qml
// Bluetooth adapter management
bluetoothManager.powered: bool
bluetoothManager.discoverable: bool
bluetoothManager.adapterAddress: string
bluetoothManager.adapterName: string
bluetoothManager.initialize(): bool
bluetoothManager.startDiscovery(): void
bluetoothManager.stopDiscovery(): void

// Device discovery
deviceDiscovery.discovering: bool
deviceDiscovery.count: int
deviceDiscovery.startDiscovery(): void
deviceDiscovery.stopDiscovery(): void
deviceDiscovery.getDevice(index): BluetoothDevice

// OBEX transfer
obexTransferService.sendFile(deviceAddress, filePath): OBEXTransfer
obexTransferService.cancelTransfer(transfer): void
```

### BluetoothDevice Properties

```qml
device.address: string
device.name: string
device.alias: string
device.paired: bool
device.connected: bool
device.trusted: bool
device.rssi: int
device.pair(): void
device.trust(): void
device.connect(): void
device.disconnect(): void
```

### OBEXTransfer Properties

```qml
transfer.filename: string
transfer.size: int64
transfer.transferred: int64
transfer.status: enum (Queued, Active, Complete, Error)
transfer.progress: int (0-100)
transfer.cancel(): void
```

## Known Issues

1. **AppArmor Manual Review**: Application uses reserved 'bluetooth' policy group which requires manual vetting for OpenStore publication (this is normal and expected for Bluetooth apps)
2. **Hardware Testing Pending**: Unable to test Bluetooth on real device from VM
3. **Emulator Limitations**: D-Bus services may not be available in desktop emulator

## Next Steps

1. Test on real hardware
2. Implement file picker for send functionality
3. Add pairing/connection management UI
4. Refine AppArmor policy
5. Add error handling and user feedback

## Git Commits

- `06b2d60` - T2.1 & T2.4: Implement BluetoothManager and fix UbuntuShape deprecation
- `f693786` - T2.2: Implement Device Discovery Service
- `f33903b` - T2.3: Implement OBEX Transfer Service
- `b7e292b` - T2.5: Implement Main Page UI with device list
- `c40075c` - Fix AppArmor policy: Replace 'unconfined' with 'bluetooth' policy group

## References

- BlueZ D-Bus API: https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc
- Qt D-Bus Documentation: https://doc.qt.io/qt-5/qtdbus-index.html
- Ubuntu Touch UI Toolkit: https://docs.ubports.com/en/latest/appdev/platform/uitk.html
