# SharePlugin Fix Summary - Sprint #007

**Date:** 2026-01-22  
**Issue:** SharePlugin crashes Lomiri shell when invoked from ContentHub

## Problem Identified

The error `"Application is not meant to be displayed"` was caused by `NoDisplay=true` in `shareplugin.desktop`. ContentHub attempted to launch the SharePlugin but failed because:

1. The desktop file declared the app should not be displayed
2. The SharePlugin has a full QML UI with MainView (requires display)
3. This contradiction caused content-hub-service to throw `std::runtime_error`
4. The exception propagated and crashed the entire Lomiri shell

## Root Cause

The SharePlugin architecture is a **visible UI application** but was marked as **headless** in the desktop file. This mismatch is incompatible with ContentHub's launch mechanism.

## Solution Applied

**Removed `NoDisplay=true` from `shareplugin.desktop`**

This single-line change makes the SharePlugin a standard visible ContentHub peer, eliminating the crash.

## Architecture Clarification

### Two Independent Bluetooth Stacks

The system uses two separate Bluetooth implementations that don't interact:

#### 1. Main App (ratatoskr) - INCOMING transfers
```
User launches app
  → Starts OBEX daemon (org.bluez.obex)
  → Registers OBEX agent on D-Bus
  → Waits for incoming file transfer requests
  → Receives files to HubIncoming folder
```

**Stack:** BlueZ OBEX via D-Bus  
**Files:** obexd.cpp, obexagent.cpp, obexagentadaptor.cpp  
**Purpose:** Receive files from other devices

#### 2. SharePlugin - OUTGOING transfers
```
User shares file via ContentHub
  → ContentHub launches SharePlugin
  → SharePlugin shows device selection UI
  → User selects target device
  → QBluetoothTransferManager sends file
```

**Stack:** Qt Bluetooth (QBluetoothTransferManager)  
**Files:** bttransfer.cpp, bttransfer.h  
**Purpose:** Send files to other devices

### Why Two Stacks?

- **OBEX daemon**: Required for receiving files on Ubuntu Touch
- **QtBluetooth**: Simpler API for sending files, better ContentHub integration
- **No conflict**: They operate independently on different channels

## Changes Made

### 1. shareplugin.desktop
```diff
 [Desktop Entry]
 Name=Bluetooth
 Exec=shareplugin
 Icon=ratatoskr
 Terminal=false
 Type=Application
 X-Ubuntu-Touch=true
 X-Lomiri-Single-Instance=true
-NoDisplay=true
```

### 2. Documentation
- Created ADR-005 explaining the decision
- Updated CLAUDE.md with new ADR reference

## Testing Plan

1. **Install updated package**
2. **Share a contact from Contacts app**
   - Verify SharePlugin appears in share menu
   - Select "ratatoskr.philipa_shareplu"
   - Should show device selection UI
3. **Select paired device**
   - Should initiate transfer
   - Should show progress
4. **Verify no crash occurs**
5. **Check logs for successful transfer**

## Expected Behavior

### Before Fix
```
Share contact → Select SharePlugin → System crashes immediately
```

### After Fix
```
Share contact → Select SharePlugin → Device list appears → Select device → Transfer starts
```

## Side Effects

### Positive
- System stability restored
- User gets visual feedback during transfers
- Device discovery UI is helpful

### Negative (Minor)
- SharePlugin now appears in app drawer
- Launching directly shows empty screen (no files to share)

### Mitigation for Direct Launch
The SharePlugin expects ContentHub to provide files. When launched directly:
- Shows empty file list (expected)
- User can close with X button
- No crash or error

## Cleanup Opportunities

The following folders contain only placeholder files and can be removed:
- `ratatoskr/plugins/` - Contains only empty CMakeLists.txt
- `ratatoskr/qml/` - Contains template Main.qml not used by app

These are remnants from the project template and serve no purpose.

## References

- ADR-005: `docs/ADR-005-shareplugin-nodisplay-issue.md`
- Test Report: `docs/sprint-006/SharePlugin_test_report-2.md`
- Journal Logs: `docs/sprint-006/journal_cropped-20260122.txt`
