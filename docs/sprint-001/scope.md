# Sprint 001 Scope

**Sprint**: 001  
**Start Date**: 2025-12-18  
**Target Version**: 0.0.1-dev  
**Goal**: Establish foundation by adapting Ian L.'s ubtd-20.04 as Ratatoskr base

## Objectives

The primary objective of Sprint 001 is to establish a working foundation for Ratatoskr by adapting the proven ubtd-20.04 codebase, rebranding it, and verifying it builds and deploys correctly on Ubuntu Touch 20.04.

## Deliverables

### 1. Codebase Integration
- [ ] Clone/integrate ubtd-20.04 source code into ratatoskr project structure
- [ ] Preserve original attribution and licensing information
- [ ] Document source provenance in README

### 2. Rebranding
- [ ] Rename app from "ubtd" to "Ratatoskr"
- [ ] Update `manifest.json.in`:
  - Package name: `ratatoskr.philipa`
  - Title: "Ratatoskr"
  - Description: "Bluetooth file transfer with focus on car kit VCF sync"
  - Maintainer information
- [ ] Update desktop file (`ratatoskr.desktop.in`)
- [ ] Update AppArmor profile (`ratatoskr.apparmor`)
- [ ] Replace/update application icon/assets

### 3. Build System Configuration
- [ ] Adapt build system for clickable
- [ ] Update `CMakeLists.txt` if necessary
- [ ] Configure `clickable.yaml` correctly
- [ ] Ensure all dependencies are properly declared

### 4. Source File Headers
- [ ] Apply source file header template from `templates/header.cpp`
- [ ] Add version history entries:
  - Version 0.0.1 - Initial adaptation from ubtd-20.04
  - Date: 2025-12-18
- [ ] Preserve original copyright notices where appropriate

### 5. Testing & Verification
- [ ] Build successfully with `clickable build`
- [ ] Deploy to device/emulator with `clickable install`
- [ ] Verify app launches without crashes
- [ ] Basic smoke test: UI appears correctly

### 6. Documentation
- [ ] Create architecture overview document
- [ ] Document key components:
  - BtTransfer class (outgoing transfers)
  - Obexd class (incoming transfers)
  - ObexAgent (D-Bus agent)
- [ ] Update main README.md with project information
- [ ] Document known limitations from ubtd-20.04

## Technical Tasks

### Source Code Organization
```
ratatoskr/
├── qml/                  # QML UI files
├── plugins/              # C++ plugins
│   └── BluetoothTransfer/
│       ├── bttransfer.cpp/.h      # Outgoing transfers (Qt)
│       ├── obexd.cpp/.h           # Incoming transfers (D-Bus)
│       ├── obexagent.cpp/.h       # OBEX agent
│       ├── transfer.cpp/.h        # Transfer model
│       └── plugin.cpp             # QML plugin registration
├── main.cpp
├── CMakeLists.txt
├── clickable.yaml
├── manifest.json.in
├── ratatoskr.desktop.in
└── ratatoskr.apparmor
```

### Key Components to Understand

1. **BtTransfer** (`bttransfer.cpp`):
   - Uses Qt's `QBluetoothTransferManager`
   - Manages outgoing file queue
   - Handles OBEX OPP transfers

2. **Obexd** (`obexd.cpp`):
   - D-Bus interface to `org.openobex`
   - Registers OBEX agent for incoming transfers
   - Provides QAbstractListModel for transfer history

3. **ObexAgent** (`obexagent.cpp`):
   - D-Bus adaptable object
   - Authorizes incoming transfer requests
   - Manages file paths and metadata

### Dependencies to Verify
- Qt 5.12.9 modules:
  - QtCore
  - QtQml
  - QtQuick
  - QtBluetooth
  - QtDBus
- System:
  - bluez-obexd daemon
  - BlueZ stack

## Out of Scope

The following are explicitly **not** included in Sprint 001:

- ❌ New features or functionality
- ❌ UI/UX improvements or redesign
- ❌ Contacts integration (VCF export)
- ❌ Device pairing enhancements
- ❌ Performance optimizations
- ❌ Extensive testing beyond smoke tests
- ❌ User documentation or help system
- ❌ Translations/internationalization updates

## Success Criteria

Sprint 001 is considered successful when:

1. ✅ Code builds without errors using `clickable build`
2. ✅ App installs on Ubuntu Touch 20.04 device/emulator
3. ✅ App launches and displays main UI
4. ✅ All source files have proper headers with version information
5. ✅ Documentation clearly describes architecture and components
6. ✅ No regressions in basic functionality from original ubtd

## Risks & Mitigations

### Risk 1: Build System Incompatibilities
**Impact**: High  
**Probability**: Medium  
**Mitigation**: Review clickable documentation; compare with other UT 20.04 apps

### Risk 2: D-Bus API Changes in UT 20.04
**Impact**: High  
**Probability**: Low  
**Mitigation**: Ian's 2024 update suggests APIs are stable; test early

### Risk 3: Missing Dependencies
**Impact**: Medium  
**Probability**: Low  
**Mitigation**: Carefully review manifest permissions and dependencies

### Risk 4: AppArmor Policy Issues
**Impact**: Medium  
**Probability**: Medium  
**Mitigation**: Reference working ubtd AppArmor profile; adjust as needed

## Notes

- Original work by Michael Zanetti (Canonical)
- Adapted by Ian L. (floop2/fourloop2002) for UT 16.04 and 20.04
- Our adaptation: Ratatoskr by Philipa

## Next Sprint Preview

Sprint 002 will likely focus on:

- Understanding and testing Bluetooth device discovery
- Exploring contacts API for VCF export
- Initial car kit compatibility testing
- UI/UX review and minor improvements
