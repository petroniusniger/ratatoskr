# Sprint #003 Plan

**Target Version:** 0.0.3  
**Created:** 2025-12-25  
**Status:** Ready for Implementation  
**Research:** Completed 2025-12-25

## Executive Summary

Sprint #003 focuses on resolving critical blocking issues discovered during Sprint #002 testing. While the application architecture and UI implementation from Sprint #002 are solid, several runtime issues prevent the app from functioning on real hardware.

## Issues from Sprint #002

### Critical Issues (Blocking)

1. **ISSUE-001: AppArmor Bluetooth Access Denied**
   - **Severity:** Critical
   - **Description:** Main app cannot register OBEX agent due to AppArmor policy denial
   - **Evidence:** `org.freedesktop.DBus.Error.AccessDenied` in main app logs
   - **Impact:** Core Bluetooth functionality completely blocked
   - **Root Cause:** TBD (requires research into Ubuntu Touch AppArmor requirements)

2. **ISSUE-002: SharePlugin System Crash**
   - **Severity:** Critical
   - **Description:** Phone instantly reboots when SharePlugin is invoked from Contacts app
   - **Evidence:** 
     - `QObject::~QObject: Timers cannot be stopped from another thread`
     - `lost connection to Mir server`
   - **Impact:** SharePlugin completely unusable; potential data loss risk for users
   - **Root Cause:** Threading/lifecycle management issue in SharePlugin implementation

3. **ISSUE-003: SharePlugin Icon Not Displayed**
   - **Severity:** High
   - **Description:** SharePlugin shows blank icon in content-hub share menu
   - **Evidence:** `Failed to get image from provider: image://content-hub/ratatoskr.philipa_shareplugin_251220171217`
   - **Impact:** Poor user experience; app not easily identifiable in share menu
   - **Root Cause:** Incorrect icon path in shareplugin.desktop file

### High Priority Issues (Non-Blocking)

4. **ISSUE-004: Bluetooth Discoverability**
   - **Severity:** High
   - **Description:** Phone not discoverable to other Bluetooth devices (laptop test)
   - **Impact:** Cannot establish new pairings while app is intended to run
   - **Root Cause:** TBD (may be OBEX server profile not registered, or system-level issue)

## Sprint #003 Objectives

### Primary Goals

1. **Fix AppArmor Policy** (ISSUE-001)
   - Research Ubuntu Touch AppArmor requirements for Bluetooth access
   - Identify proper policy configuration or alternative approaches
   - Test and verify Bluetooth stack access on real hardware

2. **Fix SharePlugin Crash** (ISSUE-002)
   - Debug threading model in SharePlugin implementation
   - Review QObject lifecycle management
   - Implement proper thread-safe cleanup
   - Test SharePlugin invocation without system crash

3. **Fix SharePlugin Icon** (ISSUE-003)
   - Correct icon path in shareplugin.desktop
   - Test icon display in content-hub share menu

### Secondary Goals

4. **Investigate Bluetooth Discoverability** (ISSUE-004)
   - Determine if issue is app-related or system-level
   - Implement OBEX server profile registration if needed
   - Document findings and resolution approach

## Research Findings Summary

Research completed 2025-12-25. See `docs/sprint-003/research-findings.md` for full details.

**Key Findings:**

1. **AppArmor Issue**: Code uses session bus instead of system bus for OBEX. Simple fix: change `sessionBus()` to `systemBus()` in obexd.cpp
2. **SharePlugin Crash**: Threading violation - QObjects with active timers destroyed incorrectly. Fix: proper cleanup + switch to QQmlApplicationEngine
3. **Icon Issue**: Wrong path format. Fix: change `Icon=assets/logo.svg` to `Icon=ratatoskr` in desktop file
4. **Discoverability**: Needs investigation - may be system-level, not app-specific

## Technical Implementation Plan

### Task 1: Fix SharePlugin Icon (Quick Win)

**Priority:** High  
**Estimated Time:** 15 minutes  
**Complexity:** Trivial

**Changes:**
- Edit `ratatoskr/shareplugin/shareplugin.desktop`
- Change `Icon=assets/logo.svg` to `Icon=ratatoskr`

**Testing:**
- Rebuild and install on device
- Open Contacts → Share contact
- Verify icon displays in share menu

---

### Task 2: Fix AppArmor Bluetooth Access

**Priority:** Critical  
**Estimated Time:** 1-2 hours  
**Complexity:** Low

**Root Cause:** OBEX daemon accessed via session bus, but AppArmor policy grants system bus access.

**Changes:**

In `ratatoskr/obexd.cpp`:
```cpp
// Line 22: Change from sessionBus() to systemBus()
Obexd::Obexd(QObject *parent) :
    QAbstractListModel(parent),
    m_dbus(QDBusConnection::systemBus()),  // Was: sessionBus()
    m_manager("org.bluez.obex", "/org/bluez/obex", 
              "org.bluez.obex.AgentManager1", m_dbus)
```

**Testing:**
- Build and deploy to device
- Launch main app
- Check logs for successful agent registration (no AccessDenied error)
- Test file receiving functionality
- Monitor for AppArmor denials

**Success Criteria:**
- No `org.freedesktop.DBus.Error.AccessDenied` in logs
- OBEX agent registers successfully
- File receiving works

---

### Task 3: Fix SharePlugin Crash (Critical)

**Priority:** Critical  
**Estimated Time:** 6-9 hours  
**Complexity:** Medium-High

**Root Cause:** Threading violation - QObjects with active timers/threads destroyed from different thread context when ContentHub terminates plugin.

**Sub-task 3.1: Add Proper Cleanup**

In `ratatoskr/shareplugin/Main.qml`, add:
```qml
Connections {
    target: Qt.application
    onAboutToQuit: {
        scheduleRestart.stop()
        btModel.running = false
        btModel.continuousDiscovery = false
    }
}
```

**Sub-task 3.2: Add BtTransfer Destructor**

Create new file `ratatoskr/shareplugin/bttransfer.cpp` additions:
```cpp
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

Update `bttransfer.h` to declare destructor:
```cpp
~BtTransfer();
```

**Sub-task 3.3: Remove Problematic Timer Pattern**

In `Main.qml`, replace:
```qml
Timer { id: scheduleRestart; interval: 1000; onTriggered: btModel.running = true; }
Component.onCompleted: scheduleRestart.start()
```

With safer initialization:
```qml
BluetoothDiscoveryModel {
    id: btModel
    running: false
    
    Component.onCompleted: {
        Qt.callLater(function() {
            running = true
        })
    }
}
```

**Sub-task 3.4: Switch to QQmlApplicationEngine**

In `ratatoskr/shareplugin/main.cpp`:
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

In `Main.qml`, change root element:
```qml
// Change from MainView to Window
import QtQuick.Window 2.2

Window {
    id: root
    visible: true
    // ... rest of properties
}
```

**Testing:**
- Build and deploy to device
- Open Contacts app
- Test share invocation
- Test canceling before selection
- Test completing transfer
- Test multiple invocations
- Monitor logs for threading errors
- Verify no system crashes

**Success Criteria:**
- No `QObject::~QObject: Timers cannot be stopped` errors
- No system crashes or reboots
- SharePlugin cleanly exits when dismissed
- Multiple invocations work without issues

---

### Task 4: Investigate Bluetooth Discoverability

**Priority:** Medium  
**Estimated Time:** 2-3 hours  
**Complexity:** Low (investigation only)

**Phase 1: System-Level Testing**

1. On device: Settings → Bluetooth → Enable "Visible to nearby devices"
2. Attempt pairing laptop ↔ phone through system settings
3. Document whether pairing succeeds
4. If succeeds: Issue is not Ratatoskr-related
5. If fails: Investigate OBEX Server registration need

**Phase 2: OBEX Server Implementation (if needed)**

Only proceed if Phase 1 confirms app-specific issue.

Add to `obexd.cpp`:
```cpp
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
        "opp",
        properties
    );
    
    if (!reply.isValid()) {
        qWarning() << "Failed to register OBEX server:" << reply.error();
    }
}
```

**Testing:**
- If implemented, test discoverability with server registered
- Verify remote devices can see file transfer capability
- Test file transfer initiation from remote device

**Success Criteria:**
- Understanding of whether issue is app-specific or system-level
- If app-specific and fixable: working discoverability
- If system-level: documented limitation

---

## Implementation Timeline

**Week 1:**
- Day 1: Task 1 (Icon fix) + Task 2 (AppArmor fix) + Test on device
- Day 2-3: Task 3.1-3.2 (Cleanup + Destructor) + Test
- Day 4-5: Task 3.3-3.4 (Engine refactor) + Comprehensive testing
- Day 6: Task 4 (Discoverability investigation)
- Day 7: Final integration testing + documentation

**Estimated Total:** 15-23 hours spread over one week

---

## Testing Strategy

### Unit Testing
- Each fix tested independently before integration
- Use git commits to isolate changes
- Document test results for each fix

### Integration Testing
- Test all fixes together on real hardware
- Multiple invocation cycles for SharePlugin
- Extended runtime testing for stability
- Log monitoring throughout

### Test Devices
- Primary: Ubuntu Touch device (your phone)
- Secondary: Emulator (for basic validation only)
- External: Laptop for Bluetooth pairing tests

### Test Scenarios

**Main App:**
1. App startup
2. Bluetooth adapter detection
3. Device discovery
4. File receiving
5. Log verification (no AppArmor denials)

**SharePlugin:**
1. Invocation from Contacts app
2. Cancel before selection
3. Select device and transfer
4. Multiple consecutive invocations
5. System stability after operations

**Integration:**
1. Main app running + SharePlugin invocation
2. File transfer end-to-end
3. Multiple file types (VCF primary focus)
4. Error handling

---

## Success Criteria

- [ ] Main app successfully registers OBEX agent on real hardware (no AccessDenied)
- [ ] SharePlugin can be invoked without causing system crash or reboot
- [ ] SharePlugin icon displays correctly in share menu
- [ ] SharePlugin destructor properly cleans up resources
- [ ] No threading violation errors in logs
- [ ] System remains stable after multiple SharePlugin uses
- [ ] Bluetooth discoverability issue understood and documented
- [ ] All fixes tested on real hardware (not just emulator)
- [ ] Version updated to 0.0.3 in all relevant files

## Out of Scope

- New features
- UI/UX improvements
- Additional file type support
- Performance optimizations
- Translation updates (unless required by fixes)

## Risk Management

### Technical Risks

| Risk | Probability | Impact | Mitigation |
|------|------------|---------|------------|
| System bus doesn't solve AppArmor issue | Low | High | Fallback: investigate custom AppArmor profile |
| SharePlugin refactor introduces new bugs | Medium | High | Incremental changes, thorough testing, git rollback ready |
| Engine switch breaks QML features | Low | Medium | Test all UI functionality, keep QQuickView version in git |
| Discoverability unfixable at app level | Medium | Low | Document as system limitation, not blocker |
| Multiple fixes interact negatively | Low | High | Test each fix independently first |

### Schedule Risks

| Risk | Probability | Impact | Mitigation |
|------|------------|---------|------------|
| SharePlugin testing takes longer | Medium | Medium | Allocate extra time, prioritize core functionality |
| Device unavailable for testing | Low | High | Use emulator for development, batch device tests |
| New issues discovered during testing | Medium | Medium | Timebox investigation, defer to Sprint 004 if needed |

### Rollback Plan

Each task committed separately to git:
- **Task 1 (Icon)**: Revert single file if needed
- **Task 2 (AppArmor)**: Revert to sessionBus() if systemBus() fails
- **Task 3 (SharePlugin)**: Revert in stages - destructor, cleanup, engine
- **Task 4 (Discoverability)**: Investigation only, no code changes to revert

## Dependencies

- Access to real Ubuntu Touch device for testing
- Ubuntu Touch AppArmor documentation
- Content-Hub plugin development documentation

### External
- Access to Ubuntu Touch device for testing (REQUIRED)
- Laptop with Bluetooth for discoverability testing (OPTIONAL)
- Stable Ubuntu Touch system on device (REQUIRED)

### Documentation
- BlueZ D-Bus API documentation
- Qt QML lifecycle documentation  
- Ubuntu Touch ContentHub documentation

### Tools
- clickable (build system)
- git (version control)
- Device logging tools (adb logcat, journalctl)

## Documentation Updates

### Files to Update
- `docs/sprint-003/sprint-003-plan.md` (this file)
- `docs/sprint-003/research-findings.md` (completed)
- `docs/sprint-003/test-results.md` (to be created during testing)
- `docs/sprint-003/completion-summary.md` (to be created at end)
- `CLAUDE.md` - Update ADR index with any new ADRs
- Source file headers - Update modification dates to 2025-12-25+ and version to 0.0.3

### ADRs to Create (if needed)
- ADR-003: SharePlugin Lifecycle Management (if architecture change significant)
- ADR-004: OBEX Bus Selection (system vs session)

## Notes

- This sprint is entirely focused on bug fixes from Sprint #002
- No new functionality will be added
- Testing must be performed on real hardware, as emulator doesn't catch these issues
- Sprint #002 provided solid foundation; Sprint #003 makes it actually work

## Next Steps

1. ✅ **COMPLETED:** Merge sprint-002 branch to master
2. ✅ **COMPLETED:** Conduct research tasks
3. ✅ **COMPLETED:** Update this plan with detailed technical approach
4. ✅ **COMPLETED:** Create sprint-003 branch
5. **READY:** Begin implementation (awaiting approval)
