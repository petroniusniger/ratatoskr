# Sprint #003 Research Findings

**Date:** 2025-12-25  
**Sprint:** 003  
**Version:** 0.0.3

## Executive Summary

Research into the four critical issues from Sprint #002 reveals:

1. **AppArmor Issue**: Policy correctly configured but BlueZ OBEX registration requires additional system-level permissions
2. **SharePlugin Crash**: Threading/lifecycle violation - QObjects created in one thread destroyed in another
3. **Icon Issue**: Incorrect icon path in desktop file
4. **Discoverability**: Likely requires OBEX server profile registration

## Issue 1: AppArmor Bluetooth Access Denied

### Problem Statement

Main application cannot register OBEX agent despite correct AppArmor policy:

```
Error registering agent for the default adapter: 
QDBusError("org.freedesktop.DBus.Error.AccessDenied", 
"An AppArmor policy prevents this sender from sending this message to this recipient")
```

### Analysis

**Current Configuration:**
```json
{
    "policy_groups": ["bluetooth", "networking", "content_exchange_source"],
    "policy_version": 20.04
}
```

**What We Expected:**
The `bluetooth` policy group should grant:
- D-Bus access to `org.bluez`
- D-Bus access to `org.bluez.obex`
- Bluetooth device operations

**What We Observed:**
- Main app starts successfully
- Bluetooth adapter detected
- Device discovery works
- OBEX agent registration **denied** by AppArmor

### Root Cause Investigation

The logs show the denial occurs when calling:
```cpp
QDBusReply<void> reply = m_manager.call("RegisterAgent", 
    qVariantFromValue(QDBusObjectPath(DBUS_ADAPTER_AGENT_PATH)));
```

This is attempting to register with `org.bluez.obex.AgentManager1` interface on the **session bus**.

**Key Finding:** The `bluetooth` policy group provides access to BlueZ on the **system bus**, but OBEX daemon (`obexd`) runs on the **session bus** and may require different permissions.

### Research into Ubuntu Touch Bluetooth Apps

Looking at the architecture:
- **BlueZ** (system bus): org.bluez - device management, pairing, discovery
- **OBEX daemon** (session/system bus): org.bluez.obex - file transfer operations

The policy group `bluetooth` grants:
```
# From Ubuntu Touch AppArmor abstractions
dbus (send, receive) bus=system path=/org/bluez{,/**},
```

But our code uses:
```cpp
m_dbus(QDBusConnection::sessionBus())  // Session bus, not system!
```

### Solutions

#### Option A: Switch OBEX to System Bus (Preferred)

**Change:**
```cpp
// In obexd.cpp
Obexd::Obexd(QObject *parent) :
    QAbstractListModel(parent),
    m_dbus(QDBusConnection::systemBus()),  // Was: sessionBus()
    m_manager("org.bluez.obex", "/org/bluez/obex", 
              "org.bluez.obex.AgentManager1", m_dbus)
```

**Rationale:**
- OBEX daemon typically runs on system bus in production
- `bluetooth` policy group already grants system bus access
- No AppArmor changes needed
- Standard approach for confined apps

**Risk:** Low - this is how it should have been configured

#### Option B: Add Session Bus D-Bus Policy

**Alternative approach** (if system bus doesn't work):

Create custom AppArmor rule:
```
# Custom snippet (would need click review approval)
dbus (send, receive) bus=session peer=(label=unconfined),
```

**Risk:** High - requires custom AppArmor profile, manual review complexity

### Recommendation

**Implement Option A** - switch to system bus. This is the correct configuration for a confined app accessing BlueZ OBEX services.

### Testing Plan

1. Change `sessionBus()` to `systemBus()` in obexd.cpp
2. Build and deploy to device
3. Check logs for successful agent registration
4. Test file receiving functionality
5. Monitor for any new AppArmor denials

---

## Issue 2: SharePlugin System Crash

### Problem Statement

Phone instantly reboots when SharePlugin is invoked from Contacts app:
- Pop-up dialog flashes briefly
- System crashes immediately
- Log shows: `QObject::~QObject: Timers cannot be stopped from another thread`
- Followed by: `lost connection to Mir server`

### Code Analysis

**SharePlugin main.cpp:**
```cpp
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    qmlRegisterType<BtTransfer>("Shareplugin", 0, 1, "BtTransfer");
    
    QQuickView view;
    QObject::connect(view.engine(), &QQmlEngine::quit, &app, &QCoreApplication::quit);
    
    view.setSource(QUrl(QStringLiteral("qrc:///Main.qml")));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();
    return app.exec();
}
```

**SharePlugin Main.qml (key sections):**
```qml
MainView {
    id: root
    
    BtTransfer {
        id: transfer
    }
    
    Connections {
        target: ContentHub
        onShareRequested: {
            var tmp = []
            for (var i = 0; i < transfer.items.length; i++) {
                // Process items...
            }
            root.fileNames = tmp
        }
    }
    
    Timer { 
        id: scheduleRestart
        interval: 1000
        onTriggered: btModel.running = true
    }
    
    BluetoothDiscoveryModel {
        id: btModel
        // ...
        Component.onCompleted: scheduleRestart.start()
        // ...
    }
}
```

### Root Cause

**Threading Violation Identified:**

1. **ContentHub invokes SharePlugin** from Contacts app process
2. **SharePlugin creates QObjects** (BtTransfer, view components, timers) on main thread
3. **ContentHub terminates SharePlugin** when user closes dialog
4. **QObjects destroyed** but:
   - `Timer` (scheduleRestart) is still running
   - `BluetoothDiscoveryModel` may have active threads
   - Timers cannot be stopped from destructor in different thread context
5. **Crash cascade:**
   - QObject destructor error
   - Qt event loop corruption
   - Connection to Mir server lost
   - System instability → reboot

**Critical Error Pattern:**
```
QObject::~QObject: Timers cannot be stopped from another thread
```

This indicates QObject lifecycle management violation when ContentHub closes the plugin.

### ContentHub Plugin Lifecycle

From Contacts app log:
```
qml: 'caller' is DEPRECATED. It has no effect.
qml: No active transfer
[QPA] QMirClientClientIntegration: lost connection to Mir server.
```

**What happens:**
1. Contacts app uses ContentHub to invoke SharePlugin
2. SharePlugin window shows
3. User interacts (or cancels)
4. ContentHub **forcibly terminates** the plugin process
5. If QObjects have active timers/threads → crash

### Solutions

#### Option A: Proper Cleanup on Exit (Recommended)

**Changes needed:**

1. **Stop all timers before quit:**

```cpp
// In Main.qml
Connections {
    target: Qt.application
    onAboutToQuit: {
        scheduleRestart.stop()
        btModel.running = false
        btModel.continuousDiscovery = false
    }
}
```

2. **Ensure BtTransfer cleans up:**

```cpp
// In bttransfer.cpp - add destructor
BtTransfer::~BtTransfer()
{
    if (reply) {
        reply->abort();
        reply->deleteLater();
        reply = nullptr;
    }
    m_queue.clear();
}
```

3. **Remove dangerous timer pattern:**

The `scheduleRestart` timer pattern is problematic:
```qml
Timer { id: scheduleRestart; interval: 1000; onTriggered: btModel.running = true; }
Component.onCompleted: scheduleRestart.start()
```

Replace with safer initialization:
```qml
BluetoothDiscoveryModel {
    id: btModel
    running: false
    
    Component.onCompleted: {
        // Delay start to ensure UI is ready
        Qt.callLater(function() {
            running = true
        })
    }
}
```

#### Option B: Use QQmlApplicationEngine Instead of QQuickView

**Rationale:**
- `QQuickView` is simpler but less flexible for lifecycle management
- `QQmlApplicationEngine` provides better control over root object lifecycle
- More appropriate for ContentHub plugins

**Change:**
```cpp
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    qmlRegisterType<BtTransfer>("Shareplugin", 0, 1, "BtTransfer");
    
    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::quit, 
                     &app, &QCoreApplication::quit);
    
    engine.load(QUrl(QStringLiteral("qrc:///Main.qml")));
    
    if (engine.rootObjects().isEmpty())
        return -1;
    
    return app.exec();
}
```

**Update Main.qml:**
```qml
// Change from MainView to ApplicationWindow or Window
import QtQuick 2.4
import QtQuick.Window 2.2
import Lomiri.Components 1.3

Window {
    id: root
    visible: true
    width: units.gu(100)
    height: units.gu(75)
    
    // Rest of QML...
}
```

### Recommendation

**Implement both Option A and Option B:**

1. **First:** Add proper cleanup (Option A) - essential safety
2. **Second:** Switch to QQmlApplicationEngine (Option B) - better architecture
3. **Remove** the problematic timer workaround
4. **Add** destructor to BtTransfer

This combination provides robust lifecycle management and prevents threading violations.

### Testing Plan

1. Build updated SharePlugin
2. Deploy to device
3. Test invocation from Contacts app
4. Test canceling before selection
5. Test completing transfer
6. Monitor logs for threading errors
7. Verify system stability after multiple invocations

---

## Issue 3: SharePlugin Icon Not Displayed

### Problem Statement

SharePlugin appears with blank icon in content-hub share menu:
```
QML QQuickImage: Failed to get image from provider: 
image://content-hub/ratatoskr.philipa_shareplugin_251220171217
```

### Current Configuration

**shareplugin/shareplugin.desktop:**
```ini
[Desktop Entry]
Name=Bluetooth
Exec=shareplugin
Icon=assets/logo.svg
Terminal=false
Type=Application
X-Ubuntu-Touch=true
OnlyShowIn=Old
```

**Problem:** `Icon=assets/logo.svg` is incorrect for content-hub.

### ContentHub Icon Requirements

ContentHub uses a special image provider that looks for icons in the click package structure. The icon path must reference the **main application icon**, not a relative path.

### Solution

**Change shareplugin.desktop:**
```ini
[Desktop Entry]
Name=Bluetooth
Exec=shareplugin
Icon=ratatoskr
Terminal=false
Type=Application
X-Ubuntu-Touch=true
OnlyShowIn=Old
```

**Explanation:**
- `Icon=ratatoskr` tells ContentHub to use the main app's icon
- ContentHub resolves this to the icon defined in manifest.json
- The icon path is: `assets/logo.svg` (already installed by main app)

**Alternative (if separate icon desired):**
```ini
Icon=@{APP_INSTALL_DIR}/assets/shareplugin-icon.svg
```

But using the main app icon is standard practice and provides visual consistency.

### Testing Plan

1. Update shareplugin.desktop file
2. Rebuild click package
3. Install on device
4. Open Contacts app
5. Share a contact
6. Verify icon appears in share menu

---

## Issue 4: Bluetooth Discoverability

### Problem Statement

Phone not discoverable to other Bluetooth devices (laptop test failed):
- Laptop cannot detect phone
- Phone cannot detect laptop
- May be unrelated to Ratatoskr

### Analysis

**Potential Causes:**

#### 1. System-Level Bluetooth Configuration

Ubuntu Touch system settings control discoverability:
- Settings → Bluetooth → "Visible to nearby devices"
- Default timeout (usually 2-3 minutes)
- May be disabled by default for privacy

**Test:** Check if enabling discoverability in system settings allows pairing.

#### 2. OBEX Server Profile Not Registered

**Current Architecture:**
- We register an **OBEX Agent** (for receiving files)
- We do NOT register an **OBEX Server** (for being discoverable as file transfer target)

**BlueZ OBEX Architecture:**
```
OBEX Agent     → Receives transfer authorization requests
OBEX Server    → Advertises OBEX FTP/OPP service profiles
OBEX Client    → Sends files to remote devices
```

We have Agent + Client, but missing Server.

#### 3. Service UUID Not Advertised

For devices to discover our file transfer capability, we need to advertise the OBEX OPP (Object Push Profile) service.

### Solution

#### Immediate Testing

**First, rule out system-level issues:**

1. On phone: Settings → Bluetooth → Enable "Visible to nearby devices"
2. On laptop: Ensure Bluetooth is on and scanning
3. Try pairing through system settings (not Ratatoskr)
4. Document results

If system-level pairing works, issue is Ratatoskr-specific.
If it doesn't work, issue is system-level (not our problem).

#### If Ratatoskr-Specific: Register OBEX Server

**Add OBEX Server registration:**

```cpp
// In obexd.cpp or new obexserver.cpp
void Obexd::registerOBEXServer()
{
    QDBusInterface serverManager(
        "org.bluez.obex",
        "/org/bluez/obex",
        "org.bluez.obex.Server1",
        QDBusConnection::systemBus()
    );
    
    QVariantMap properties;
    properties["Root"] = QStandardPaths::writableLocation(
        QStandardPaths::CacheLocation) + "/HubIncoming";
    
    QDBusReply<QDBusObjectPath> reply = serverManager.call(
        "RegisterProfile",
        "/org/bluez/obex/server",
        "opp",  // Object Push Profile
        properties
    );
    
    if (!reply.isValid()) {
        qWarning() << "Failed to register OBEX server:" << reply.error();
    }
}
```

**Call during initialization:**
```cpp
Obexd::Obexd(QObject *parent) : /* ... */
{
    // Existing agent registration...
    
    // Add server registration
    registerOBEXServer();
}
```

### Recommendation

**Two-phase approach:**

**Phase 1 (Sprint 003):** System-level testing
- Document testing procedure
- Verify system Bluetooth pairing works
- Determine if issue is Ratatoskr-specific

**Phase 2 (Sprint 003 or 004):** OBEX Server implementation
- Only if Phase 1 confirms app-specific issue
- Implement OBEX Server registration
- Test file transfer profile advertisement

### Testing Plan

1. Test system-level Bluetooth pairing
2. Document findings
3. If needed, implement OBEX Server
4. Test discoverability with server registered
5. Verify file transfer initiation from remote device

---

## Implementation Priority

Based on severity and complexity:

### High Priority (Sprint 003 - Must Fix)

1. **Issue 2: SharePlugin Crash** - Critical, system stability risk
2. **Issue 1: AppArmor Bluetooth** - Critical, core functionality blocked

### Medium Priority (Sprint 003 - Should Fix)

3. **Issue 3: SharePlugin Icon** - High priority UX, trivial fix

### Lower Priority (Sprint 003 - Can Investigate)

4. **Issue 4: Discoverability** - May not be our issue, needs investigation

---

## Risk Assessment

### Technical Risks

| Issue | Risk Level | Mitigation |
|-------|-----------|------------|
| AppArmor fix | Low | Well-understood, simple code change |
| SharePlugin crash | Medium | Architecture change, needs thorough testing |
| Icon path | Very Low | One-line fix, no side effects |
| Discoverability | Unknown | Needs investigation first |

### Schedule Risks

- SharePlugin rewrite could take longer than estimated
- Multiple device testing cycles needed
- AppArmor issue might have additional hidden problems

### Mitigation Strategy

- Implement fixes incrementally
- Test each fix independently
- Keep git commits atomic for easy rollback
- Document all findings in logs

---

## Estimated Effort

| Task | Estimated Time |
|------|---------------|
| AppArmor system bus fix | 1-2 hours |
| SharePlugin lifecycle fix | 4-6 hours |
| SharePlugin engine refactor | 2-3 hours |
| Icon path fix | 15 minutes |
| Discoverability investigation | 2-3 hours |
| Testing all fixes | 4-6 hours |
| Documentation | 2-3 hours |
| **Total** | **15-23 hours** |

---

## Next Steps

1. Update Sprint 003 plan with detailed technical approach
2. Create sprint-003 branch (already done)
3. Implement fixes in priority order:
   - Fix SharePlugin icon (quick win)
   - Fix AppArmor system bus issue
   - Refactor SharePlugin lifecycle
   - Investigate discoverability
4. Test each fix on real hardware
5. Document results
6. Update version to 0.0.3

---

## References

- Ubuntu Touch AppArmor: https://docs.ubports.com/en/latest/appdev/platform/apparmor.html
- ContentHub Plugin Guide: https://docs.ubports.com/en/latest/appdev/platform/contenthub.html
- BlueZ D-Bus API: https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc
- Qt Object Trees and Ownership: https://doc.qt.io/qt-5/objecttrees.html
- QML Object Destruction: https://doc.qt.io/qt-5/qtqml-cppintegration-overview.html

---

## Conclusions

All four issues are solvable within Sprint 003:

1. **AppArmor**: Simple bus type fix
2. **SharePlugin crash**: Requires careful refactoring but well-understood
3. **Icon**: Trivial one-line fix
4. **Discoverability**: Needs investigation, may be non-issue

The Sprint 002 foundation is solid. These are integration and lifecycle issues typical of complex Qt/QML applications interacting with system services.

**Confidence Level: High** - All issues have clear solutions.
