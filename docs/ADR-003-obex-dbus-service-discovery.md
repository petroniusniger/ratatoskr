# ADR-003: OBEX D-Bus Service Discovery and Agent Registration

**Date**: 2026-01-15  
**Status**: Accepted  
**Sprint**: #004

## Context

Investigation of the Bluetooth file transfer failure revealed critical issues with how `ratatoskr` connects to the OBEX daemon (`obexd`):

1. **Service Discovery Problem**: The app attempts to connect to `org.bluez.obex` on the system D-Bus, but `obexd` runs in the user session with a dynamic address (e.g., `:1.129`)

2. **Agent Registration Failure**: Even when `obexd` is running manually, it returns "Forbidden (0x43)" responses to PUT requests, indicating the agent isn't properly registered

3. **Service Availability**: `obexd` doesn't auto-start on boot, and the app doesn't handle its absence gracefully

Key evidence from investigation:

- D-Bus error: `"The name org.bluez.obex was not provided by any .service files"`
- OBEX logs show: `PUT(0x2), Forbidden(0x43)`
- `busctl list` shows user processes use dynamic addresses, not named services
- ratatoskr logs: `"Error registering agent for the default adapter"` on startup

## Decision

We will implement a robust OBEX service discovery and agent registration mechanism with the following components:

### 1. Dynamic Service Discovery

Instead of hardcoding `org.bluez.obex`, implement D-Bus service discovery to find the actual OBEX service:

- Query D-Bus for services implementing the OBEX interfaces
- Support both named services (`org.bluez.obex`) and dynamic addresses (`:1.xxx`)
- Use the session bus, not the system bus, since `obexd` runs as user process

### 2. Service Lifecycle Management

- Detect if `obexd` is running before attempting registration
- Attempt D-Bus activation to auto-start `obexd` if not running
- Fall back to systemd user service start if D-Bus activation fails
- Implement proper error handling and user feedback

### 3. Robust Agent Registration

- Retry agent registration with exponential backoff
- Verify registration success before claiming ready
- Handle race conditions during startup
- Properly register on correct D-Bus interface (`org.bluez.obex.AgentManager1`)

### 4. Connection Bus Change

- Switch from `QDBusConnection::systemBus()` to `QDBusConnection::sessionBus()`
- This aligns with how `obexd` runs (as user service, not system service)

## Implementation Plan

### Phase 1: Service Discovery (Priority 1)
1. Create helper method to query D-Bus for OBEX service
2. Implement fallback chain: named service → dynamic address lookup → service activation
3. Update `ObexD` constructor to use discovered service address

### Phase 2: Agent Registration (Priority 2)
1. Move to session bus connection
2. Add retry logic with backoff (3 attempts: 0s, 1s, 3s delays)
3. Verify registration by querying registered agents
4. Add comprehensive error logging

### Phase 3: User Experience (Priority 3)
1. Add status indicator for OBEX service state
2. Provide user feedback if service unavailable
3. Add option to retry connection from UI

## Technical Details

### D-Bus Service Discovery Approach

```cpp
QString ObexD::findObexService() {
    // Try named service first
    QDBusConnectionInterface *interface = 
        QDBusConnection::sessionBus().interface();
    
    if (interface->isServiceRegistered("org.bluez.obex")) {
        return "org.bluez.obex";
    }
    
    // Query for services with OBEX interfaces
    QDBusReply<QStringList> reply = interface->registeredServiceNames();
    for (const QString &service : reply.value()) {
        // Check if service implements OBEX interfaces
        if (hasObexInterface(service)) {
            return service;
        }
    }
    
    // Attempt D-Bus activation
    return activateObexService();
}
```

### Bus Change Rationale

The current code uses system bus:

```cpp
QDBusConnection m_connection = QDBusConnection::systemBus();
```

Should be:

```cpp
QDBusConnection m_connection = QDBusConnection::sessionBus();
```

This matches the `/usr/lib/systemd/user/obex.service` configuration which uses `Type=dbus` with `BusName=org.bluez.obex`, indicating a user session bus service.

## Consequences

### Positive
- App will work with `obexd` in its default configuration
- Handles service restarts gracefully
- Better error messages for troubleshooting
- More robust against system configuration variations

### Negative
- Increased complexity in initialization code
- Slightly longer startup time due to service discovery
- Need to handle more edge cases

### Neutral
- Requires testing on actual device (emulator may behave differently)
- May need AppArmor policy adjustments for D-Bus introspection

## Alternatives Considered

### Alternative 1: Start obexd from App
**Rejected**: Would require additional permissions and is outside app scope

### Alternative 2: Hardcode Dynamic Address
**Rejected**: Address changes between sessions, not reliable

### Alternative 3: Require Manual Configuration
**Rejected**: Poor user experience, defeats app purpose

## References

- Investigation report: `docs/sprint-004/obex-investigation.md`
- BlueZ OBEX API: `/usr/share/doc/bluez-obexd/`
- systemd unit file: `/usr/lib/systemd/user/obex.service`
- D-Bus specification: https://dbus.freedesktop.org/doc/dbus-specification.html
