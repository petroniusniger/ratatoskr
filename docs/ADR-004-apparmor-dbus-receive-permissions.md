# ADR-004: AppArmor D-Bus Receive Permissions for OBEX Agent

**Date:** 2026-01-20  
**Status:** Accepted  
**Context:** Sprint #004

## Context

During testing of the OBEX agent implementation, file transfers were failing with AppArmor denials. The application successfully registered as an OBEX agent on the D-Bus session bus, but when `obexd` attempted to call the `AuthorizePush` method on the agent interface, AppArmor blocked the incoming D-Bus method call.

### Error Message
```
apparmor="DENIED" operation="dbus_method_call" bus="session" 
path="/test/agent" interface="org.bluez.obex.Agent1" member="AuthorizePush"
name=":1.94" mask="receive" pid=5216 
label="ratatoskr.philipa_ratatoskr_260115153557"
```

The OBEX daemon (`obexd`) runs unconfined and attempts to call methods on our confined application's agent object. While the standard `bluetooth` policy group allows sending D-Bus messages to bluez services, it does not grant permission to receive method calls from those services.

## Decision

Due to Ubuntu Touch click package limitations, custom D-Bus receive permissions cannot be specified in the JSON apparmor manifest. The standard `bluetooth` policy group does not include permissions to receive D-Bus method calls from OBEX services.

**Solution:** Provide an AppArmor local override file (`ratatoskr.apparmor.local`) that users can manually apply after installation. The required permissions are:

```plain
dbus (receive)
     bus=session
     interface=org.bluez.obex.Agent1
     member=AuthorizePush
     peer=(label=unconfined),
```

This grants permission to receive the `AuthorizePush`, `Cancel`, and `Release` method calls from the OBEX daemon.

## Consequences

### Positive
- Provides a clear path for users who need the functionality
- Maintains security by limiting permissions to only the required D-Bus interface
- Does not compromise the base package - it can still be built and installed
- Documents the limitation for future platform improvements

### Negative
- **Blocks OpenStore publication** - manual system modifications are not allowed for store apps
- Requires technical knowledge and device access to apply the override
- Must be reapplied after each app update
- Not suitable for general users without shell access

### Neutral
- This approach is standard for power-user applications that need advanced system integration
- Similar limitations affect other Ubuntu Touch apps requiring fine-grained permissions
- The override approach is well-documented in Ubuntu Touch community

### Ubuntu Touch platform accepts custom D-Bus rules in AppArmor JSON manifests
- Changes to Ubuntu Touch's `bluetooth` policy group to include D-Bus receive permissions

## Implementation Notes

1. The base `.click` package uses only standard policy groups and will build successfully
2. The `ratatoskr.apparmor.local` file contains the necessary override rules  
3. Users must manually append the override to the generated AppArmor profile after installation
4. Installation instructions are provided in the override file header

## Long-term Path Forward

To enable OpenStore distribution, one of the following must happen:

1. Ubuntu Touch platform adds D-Bus receive permissions to the `bluetooth` policy group
2. Ubuntu Touch adds support for custom D-Bus rules in JSON apparmor manifests  
3. A new policy group (e.g., `bluetooth-agent`) is created for OBEX agent implementations

## Alternatives Considered

1. **Use `unconfined` policy template**: Implemented as a temporary workaround
   to allow first publication of a fully functional application. Working with 
   the community to determine the best way to resolve the AppArmor OBEX access
   denial will be the next priority.

2. **Request new policy group from Ubuntu Touch**: Long-term solution but uncertain timeline - should be pursued in parallel

3. **Use system bus instead of session bus**: Previously attempted in ADR-002 but architecturally incorrect for OBEX and creates different AppArmor issues

4. **Rewrite using a different Bluetooth approach**: Would require abandoning OBEX agent pattern entirely, losing the standard inter-app file transfer capability

## References

- Test results: `docs/sprint-004/test_result-260115153557.md`
- ADR-002: AppArmor Security Policy for Bluetooth Access
- ADR-003: OBEX D-Bus Service Discovery and Agent Registration
- Ubuntu Touch AppArmor documentation: https://docs.ubports.com/en/latest/appdev/platform/apparmor.html
