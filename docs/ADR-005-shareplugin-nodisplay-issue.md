# ADR-005: SharePlugin NoDisplay Issue and ContentHub Integration

**Date:** 2026-01-22 (Updated: 2026-02-05)  
**Status:** Resolved  
**Context:** Sprint #007

## Problem Statement

The SharePlugin crashes the entire Lomiri shell when invoked from ContentHub due to `NoDisplay=true` in its desktop file. The error message "Application is not meant to be displayed" triggers a `std::runtime_error` in content-hub-service that propagates and crashes Lomiri.

## Context

### Test Results (Sprint #006)

From `docs/sprint-006/journal_cropped-20260122.txt`:

```
Jan 22 10:14:03 content-hub-ser[3245]: Unable to start app 'ratatoskr.philipa_shareplugin_260121164817': Application is not meant to be displayed
Jan 22 10:14:03 lomiri[3178]: terminate called after throwing an instance of 'std::runtime_error'
Jan 22 10:14:03 lomiri[3178]:   what():  Application is not meant to be displayed
```

This causes a full system crash and restart.

### Architecture Analysis

The SharePlugin and main app use **different Bluetooth stacks**:

1. **Main app (ratatoskr)**:
   - Uses BlueZ OBEX via D-Bus (`obexd.cpp`, `obexagent.cpp`)
   - Starts OBEX daemon on launch
   - Registers OBEX agent for **incoming** file transfers
   - Handles received files

2. **SharePlugin (shareplugin)**:
   - Uses Qt's `QBluetoothTransferManager` (`bttransfer.cpp`)
   - Completely independent from OBEX daemon
   - Handles **outgoing** file transfers via ContentHub
   - Does not interact with the OBEX daemon

**They don't need to interact** - they serve different purposes (receive vs send).

### ContentHub Requirements

ContentHub peers can be:

1. **Visible applications** - Normal apps with UI (NoDisplay=false or omitted)
2. **Headless services** - Background services without UI (requires different architecture)

The current SharePlugin is a hybrid that doesn't fit either pattern:

- Has a full QML UI with MainView (requires display)
- Declares NoDisplay=true (attempts to be headless)
- This contradiction causes ContentHub to fail

## Decision

**Remove `NoDisplay=true` from shareplugin.desktop** to make it a standard visible ContentHub peer.

### Rationale

1. **Simplicity**: Minimal code change, single-line fix
2. **Safety**: Eliminates the crash completely
3. **Compatibility**: Follows ContentHub's expected patterns
4. **User Experience**: The UI provides useful feedback during transfer
5. **Low Impact**: The app appearing in the drawer is acceptable

Alternative (headless service) would require:

- Complete QML restructuring to remove MainView
- Different ContentHub API usage pattern
- Higher complexity and risk
- No significant user benefit

## Implementation

### Changes Required

1. **shareplugin.desktop**: Remove line 9 (`NoDisplay=true`)
2. **Main.qml**: Add Window root element required by QQmlApplicationEngine
3. **Testing**: Verify ContentHub integration works
4. **Cleanup**: Remove unused folders (`ratatoskr/plugins`, `ratatoskr/qml`)

### Updated Desktop File

```desktop
[Desktop Entry]
Name=Bluetooth
Exec=shareplugin
Icon=ratatoskr
Terminal=false
Type=Application
X-Ubuntu-Touch=true
X-Lomiri-Single-Instance=true
```

### Resolution Details

**Issue Discovered (2026-01-29):**  
After removing `NoDisplay=true`, the SharePlugin still failed to display. Investigation revealed that the QML file used `MainView` as the root element, which is incompatible with `QQmlApplicationEngine` used in `main.cpp`.

**Root Cause:**  
The mismatch between `QQmlApplicationEngine` (expects Window-based QML) and `MainView` (designed for `QQuickView`) prevented the UI from rendering.

**Solution Implemented:**  
Wrapped the existing `MainView` component inside a `Window` object in `Main.qml`:

```qml
Window {
    id: window
    visible: true
    width: units.gu(50)
    height: units.gu(75)

    MainView {
        id: root
        anchors.fill: parent
        // ... existing code
    }
}
```

This change made the QML compatible with `QQmlApplicationEngine` while preserving all existing functionality.

**Verification:**  
Testing confirmed the SharePlugin now displays correctly when invoked from ContentHub and successfully transfers files via Bluetooth.

## Consequences

### Positive

- SharePlugin will work with ContentHub
- System stability restored
- User gets visual feedback during transfers
- Device discovery UI is helpful

### Negative

- SharePlugin appears in app drawer (minor)
- Launching directly from drawer shows empty file list (expected behavior)

### Neutral

- OBEX daemon remains independent (already the case)
- Two Bluetooth stacks coexist (already the case)

## Follow-up Tasks

1. ~~Test SharePlugin with various file types~~ ✅ Completed
2. ~~Fix Window root element issue~~ ✅ Completed (2026-01-29)
3. ~~Add device name resolution for better UX~~ ✅ Completed (2026-02-02)
4. ~~Filter MAC-based device names~~ ✅ Completed (2026-02-05)
5. Consider adding icon to distinguish from main app
6. Document the dual-stack architecture
7. Remove obsolete folders

## References

- Test Report: `docs/sprint-006/SharePlugin_test_report-2.md`
- Journal Logs: `docs/sprint-006/journal_cropped-20260122.txt`
- ContentHub Documentation: Ubuntu Touch API Reference
