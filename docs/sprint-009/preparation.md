# Sprint #009 Preparation - Device Name Integration

Goal: Show actual device names instead of MAC addresses in SharePlugin
device list

Approach: Reuse existing code from main app

- The main app already has DeviceModel (in `ratatoskr/devicemodel.cpp`) that
  queries BlueZ D-Bus for device properties including names
- Create a similar lightweight C++ class for the SharePlugin
- Expose it to QML and populate the ListModel with proper device names

Files to review before starting:

- `ratatoskr/devicemodel.h/cpp` - Shows how to query device properties
- `ratatoskr/adapter.h/cpp` - Shows BlueZ D-Bus interface usage
- `ratatoskr/shareplugin/adapter.h/cpp` - Already exists, might be reusable
