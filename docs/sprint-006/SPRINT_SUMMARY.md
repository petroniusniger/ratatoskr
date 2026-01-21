# Sprint #006 Summary: SharePlugin Crash Fix

**Date:** 2026-01-21  
**Version:** 0.0.4  
**Status:** Implementation Complete - Ready for Testing

## Problem Analysis

The SharePlugin was causing a complete system restart (soft reboot) when selected from the Contacts app's share menu. Analysis of journal logs revealed three critical issues:

### Root Causes Identified

1. **Incorrect Application Name**
   - `Main.qml` had hardcoded `applicationName: "ubtd-20.04.fourloop2002"` from the original ubtd project
   - ContentHub expected `"ratatoskr.philipa"` to match the manifest
   - This caused a `std::runtime_error: Application is not shown in 'Lomiri'`

2. **Desktop File Visibility Issue**
   - `shareplugin.desktop` contained `OnlyShowIn=Old`
   - This prevented Lomiri from recognizing the application
   - ContentHub service failed with "Application is not shown in 'Lomiri'"

3. **Missing Single Instance Flag**
   - Desktop file lacked `X-Lomiri-Single-Instance=true`
   - Required for proper ContentHub plugin lifecycle management

### Error Sequence (from journal)
```
Jan 21 16:42:21 Failed to create Application for "ratatoskr.philipa_shareplugin_260121135941"
Jan 21 16:42:39 Unable to start app: Application is not shown in 'Lomiri'
Jan 21 16:42:39 terminate called after throwing an instance of 'std::runtime_error'
```

## Changes Implemented

### 1. `shareplugin/Main.qml`
- **Line 11:** Changed `applicationName` from `"ubtd-20.04.fourloop2002"` to `"ratatoskr.philipa"`
- **Lines 1-15:** Added standard project header with version history
- **Version:** Updated to 0.0.4

### 2. `shareplugin/shareplugin.desktop`
- **Removed:** `OnlyShowIn=Old` (line 8)
- **Added:** `X-Lomiri-Single-Instance=true`
- **Added:** `NoDisplay=true` (to hide from app drawer since this is a content hub handler)

### 3. `shareplugin/main.cpp`
- **Header:** Updated date to 2026-01-21, version to 0.0.4
- **Header:** Added modification history entry

## Build Results

Both target architectures built successfully:

- ✅ **amd64:** `ratatoskr.philipa_260121164748_amd64.click`
- ✅ **arm64:** `ratatoskr.philipa_260121164817_arm64.click`

Expected security review warnings (unconfined template, bluetooth policy group) are present but acceptable for development.

## Testing Instructions

1. Install the new arm64 package on the device
2. Open Contacts app
3. Select a contact
4. Tap the "Share" icon
5. Select "ratatoskr.philipa_shareplu" from the share menu
6. **Expected Result:** SharePlugin should launch showing Bluetooth device list
7. **Previous Result:** System restart/crash

## Technical Notes

- The `NoDisplay=true` flag ensures the SharePlugin won't appear in the app launcher
- The plugin will only be invoked via ContentHub share requests
- `X-Lomiri-Single-Instance=true` ensures only one instance runs at a time
- The applicationName must exactly match the package name in manifest.json

## Files Modified

```
ratatoskr/shareplugin/Main.qml            | +17 -1
ratatoskr/shareplugin/main.cpp            | +3 -2
ratatoskr/shareplugin/shareplugin.desktop | +3 -1
```

## Next Steps

1. Deploy and test on physical device
2. If successful, document the share workflow behavior
3. Consider refining the Bluetooth device selection UI
4. Address AppArmor confinement (future sprint)
