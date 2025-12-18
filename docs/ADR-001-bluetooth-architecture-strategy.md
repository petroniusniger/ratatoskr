# ADR-001: Bluetooth File Transfer Architecture Strategy

**Status**: Accepted  
**Date**: 2025-12-18  
**Deciders**: Development Team  
**Technical Story**: Ratatoskr Bluetooth File Transfer for Ubuntu Touch 20.04

## Context

Ratatoskr aims to provide Bluetooth file transfer capabilities for Ubuntu Touch 20.04, with two primary goals:

1. **Primary**: Send Contacts (VCF cards) to Bluetooth car kits
2. **Secondary**: Bi-directional file transfer with PCs

### Existing Solutions Analysis

Two previous implementations exist:

1. **Ian L.'s "Bluetooth file transfer" (ubtd.fourloop2002)**
   - Reported as working on Ubuntu Touch 16.04 (2022)
   - Source code available: `github.com/floop2/ubtd`
   - **UT 20.04 port available**: `github.com/floop2002/ubtd-20.04` (updated Feb 2024)
   - Architecture: Qt Bluetooth + D-Bus OBEX hybrid
   - Code is complete and well-structured

2. **Ruben Carneiro's fork (ubtd.rubencarneiro)**
   - Listed on OpenStore as non-functional
   - Source: `gitlab.com/rubencarneiro/ubtd`
   - Code appears incomplete
   - Not actively maintained

### Technical Considerations

**OBEX Protocol Requirements**:

- Car kits use OBEX Object Push Profile (OPP) for VCF reception
- Ubuntu Touch 20.04 uses BlueZ stack with `bluez-obexd` daemon
- Qt 5.12.9 provides `QBluetoothTransferManager` for OBEX OPP

**Platform Constraints**:

- Qt Bluetooth OBEX support on Linux: ✅ Full support
- D-Bus interface to bluez-obexd: Required for incoming transfers
- AppArmor security policies: Must be configured for Bluetooth access

## Decision

**We will use Ian L.'s ubtd-20.04 implementation as the foundation** for Ratatoskr, adapting and extending it rather than starting from scratch.

### Rationale

1. **Proven Working Code**: The original worked on UT 16.04; 20.04 port exists
2. **Correct Architecture**: Uses both Qt Bluetooth (standard) and D-Bus (platform-specific)
3. **Time Efficiency**: Start with ~80% complete solution
4. **OBEX Compliance**: Properly implements OPP for car kit compatibility
5. **Maintainability**: Well-structured C++/QML codebase
6. **Recent Updates**: 20.04 port updated as recently as February 2024

### Implementation Architecture

**Outgoing Transfers** (Sending to car kit/PC):

- Use Qt's `QBluetoothTransferManager`
- OBEX Object Push Profile (OPP)
- Queue-based transfer management

**Incoming Transfers** (Receiving from PC):

- D-Bus interface to `org.openobex` service
- `ObexAgent` implementation for authorization
- Automatic file management

**File Types**:

- VCF (vCard) - Primary focus for contacts
- Generic file support - For PC transfers

## Consequences

### Positive

- **Faster Development**: Building on working code vs. from scratch
- **Proven Architecture**: OBEX implementation already tested
- **Lower Risk**: Known working solution reduces unknowns
- **Good Code Quality**: Original is well-structured and documented
- **Car Kit Compatible**: Standard OBEX OPP ensures compatibility

### Negative

- **Legacy Code Debt**: Inheriting someone else's architectural decisions
- **Dependency Risk**: May need updates if UT 20.04 APIs changed
- **License Compliance**: Must respect original author attribution (Michael Zanetti, Ian L.)
- **Learning Curve**: Team must understand existing codebase

### Neutral

- **Rebranding Required**: Must change app identity from "ubtd" to "Ratatoskr"
- **Manifest Updates**: Package naming, permissions, and metadata
- **ContentHub Integration**: Already present, needs testing/validation

## Implementation Plan

### Phase 1 - Foundation (Sprint 001)
- Fork/adapt ubtd-20.04 codebase
- Rebrand to Ratatoskr
- Update build system for clickable
- Verify basic build and deployment

### Phase 2 - Primary Goal (Sprint 002-003)
- Enhance device discovery UI
- Add contacts export to VCF
- Optimize car kit compatibility
- User testing with real car kits

### Phase 3 - Secondary Goal (Sprint 004+)
- Enhance file browser/picker
- Improve incoming file handling
- PC-specific optimizations

## Alternatives Considered

### Alternative 1: Start from Scratch
- **Pros**: Clean architecture, full control, modern code
- **Cons**: 6-8 weeks additional development, reinvent OBEX, higher risk
- **Rejected**: Time inefficient, unnecessary complexity

### Alternative 2: Use Ruben Carneiro's Version
- **Pros**: Available on GitLab
- **Cons**: Incomplete, non-functional, not maintained
- **Rejected**: More work to fix than using Ian L.'s version

### Alternative 3: Qt-Only Solution (No D-Bus)
- **Pros**: Simpler, more portable
- **Cons**: Cannot receive files (Qt limitation on Linux), incomplete solution
- **Rejected**: Fails to meet secondary goal

## References

- Ian L.'s ubtd: https://github.com/floop2/ubtd
- Ian L.'s ubtd-20.04: https://github.com/floop2002/ubtd-20.04
- Qt Bluetooth Documentation: https://doc.qt.io/qt-5/qtbluetooth-index.html
- BlueZ OBEX API: http://www.bluez.org/obex-api/
- OBEX Specification: IrDA Object Exchange Protocol
