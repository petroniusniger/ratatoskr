# ADR-002: AppArmor Security Policy for Bluetooth Access

**Status**: Accepted  
**Date**: 2025-12-18  
**Deciders**: Development Team  
**Technical Story**: Fix AppArmor "unconfined" template blocking OpenStore publication

## Context

After completing Sprint 001 with the ubtd-20.04 codebase integration, automated review identified a critical security issue: the app uses the `unconfined` AppArmor template, which is not allowed for production apps.

### The Problem

```json
{
    "policy_groups": ["networking", "content_exchange_source"],
    "policy_version": 20.04,
    "template": "unconfined"
}
```

**Impact:**

- ❌ Blocks OpenStore publication (automatic review rejection)
- ❌ Security vulnerability (no AppArmor protection)
- ❌ Doesn't meet Ubuntu Touch platform standards
- ✅ Works for development/testing

### Why We Inherited This

Ian L.'s ubtd-20.04 used `unconfined` because:

1. Quick development iteration without AppArmor complexity
2. Bluetooth historically required extensive system permissions
3. ubtd was a proof-of-concept, not production software
4. Focus on functionality over security during UT 16.04→20.04 port

### Requirements

For Ratatoskr, we need:

1. **Bluetooth adapter access** - Detect and control local adapter
2. **BlueZ D-Bus access** - Communicate with org.bluez service
3. **OBEX daemon access** - Register agent with org.openobex service
4. **File system access** - Save/load transferred files
5. **Network access** - Bluetooth is technically a network protocol
6. **ContentHub integration** - Share files with other apps
7. **OpenStore compliance** - Pass automated security review

## Decision

**We will use the `bluetooth` policy group** instead of the `unconfined` template.

### Updated Configuration

**For main app (`ratatoskr.apparmor`):**

```json
{
    "policy_groups": [
        "bluetooth",
        "networking",
        "content_exchange_source"
    ],
    "policy_version": 20.04
}
```

**For shareplugin (`shareplugin/shareplugin.apparmor`):**

```json
{
    "policy_groups": [
        "bluetooth",
        "networking",
        "content_exchange"
    ],
    "policy_version": 20.04
}
```

### What This Provides

The `bluetooth` policy group grants:

- Administrative access to BlueZ 5 Bluetooth stack
- D-Bus communication with org.bluez
- Access to Bluetooth devices and sockets
- Necessary system calls for Bluetooth operations

Combined with existing policy groups:

- `networking` - Network operations
- `content_exchange_source` / `content_exchange` - ContentHub integration

## Rationale

### Why "bluetooth" Policy Group?

1. **Standard Approach**: This is how production Bluetooth apps work on Ubuntu Touch
2. **Proven Functional**: Other BT apps on OpenStore use this successfully
3. **Appropriate Permissions**: Provides exactly what we need for BlueZ/OBEX
4. **Security Maintained**: Confined profile with explicit permissions
5. **Simple Implementation**: Just add policy group, remove template
6. **Time Efficient**: 30-minute fix vs. days for custom profile

### Why Not Custom AppArmor Profile?

**Advantages of custom profile:**

- Maximum security (principle of least privilege)
- Precise control over every permission
- Best practice for security-conscious apps

**Disadvantages for Sprint 002:**

- Requires AppArmor expertise
- Time-consuming development (days, not hours)
- Iterative testing needed (complain mode → enforce mode)
- Ongoing maintenance burden
- May break with system updates
- Delays hardware testing and feature development

**Decision:** Custom profile is over-engineering for Sprint 002. The `bluetooth` policy group provides adequate security while unblocking progress. If future security analysis identifies concerns, we can create a custom profile in Sprint 003+.

### Why Not Keep "unconfined"?

**Completely unacceptable because:**

- Cannot be published to OpenStore (automatic rejection)
- Disables all AppArmor security protections
- Violates Ubuntu Touch platform standards
- Exposes users to unnecessary security risks
- Unprofessional for production software

Even for testing, `bluetooth` policy group works just as well.

## Consequences

### Positive

- ✅ **Unblocks OpenStore publication** (with manual review)
- ✅ **Maintains security** - Proper confinement with necessary permissions
- ✅ **Quick implementation** - Sprint 002 can proceed immediately
- ✅ **Standard practice** - Follows Ubuntu Touch conventions
- ✅ **Hardware testing enabled** - Can deploy to real devices
- ✅ **Community acceptable** - OpenStore reviewers expect this approach

### Negative

- ⚠️ **Requires Manual Review** - "bluetooth" is reserved policy group
  - *Mitigation*: This is normal for Bluetooth apps; provide clear justification
- ⚠️ **Broader permissions than minimal** - Not principle of least privilege
  - *Mitigation*: Standard for this app category; acceptable trade-off
  - *Future*: Can create custom profile if security audit requires it

### Neutral

- **OpenStore Publication Process**: Must request manual review
  - Justification: "Bluetooth file transfer app requires BlueZ stack access for OBEX file sending/receiving"
  - Expected outcome: Approval (standard for Bluetooth apps)

## Implementation

### Changes Required

1. Edit `ratatoskr/ratatoskr.apparmor`:
   - Add `"bluetooth"` to policy_groups array
   - Remove `"template": "unconfined"` line

2. Edit `ratatoskr/shareplugin/shareplugin.apparmor`:
   - Add `"bluetooth"` to policy_groups array
   - Remove `"template": "unconfined"` line

3. Build and verify:

   ```bash
   clickable build
   # Should complete without security template errors
   ```

4. Test on hardware:
   - Deploy to Ubuntu Touch device
   - Verify Bluetooth operations work
   - Monitor logs for AppArmor denials
   - Document any issues

### Validation Criteria

- [ ] Build passes automated review (no "unconfined" errors)
- [ ] App installs on real device
- [ ] Bluetooth adapter detected
- [ ] File receiving works
- [ ] File sending works
- [ ] No AppArmor denials in logs
- [ ] ContentHub integration functional

### Rollback Plan

If `bluetooth` policy group proves insufficient:

1. Document specific AppArmor denials from logs
2. Create ADR-003 for custom profile decision
3. Develop minimal custom AppArmor profile
4. Test iteratively (complain → enforce mode)
5. Update documentation

## Monitoring

After deployment, monitor for AppArmor denials:

```bash
# On device
dmesg | grep DENIED
journalctl | grep apparmor
```

If denials occur:

- Document the denied operation
- Assess if functionality is impacted
- Evaluate if custom profile is needed
- Plan for Sprint 003 if necessary

## Future Considerations

### Custom AppArmor Profile (Optional)

If Sprint 003+ requires maximum security hardening:

**Profile elements needed:**

```
# D-Bus access
dbus (send, receive) bus=system name=org.bluez,
dbus (send, receive) bus=session name=org.openobex,

# Bluetooth device access
/sys/class/bluetooth/ r,
/sys/devices/**/bluetooth/** r,

# OBEX file paths
@{HOME}/.cache/@{APP_PKGNAME}/ rw,
@{HOME}/.local/share/@{APP_PKGNAME}/ rw,

# Capabilities
capability net_bind_service,
```

**Process:**

1. Start with `bluetooth` abstractions
2. Profile actual app behavior
3. Refine in complain mode
4. Test in enforce mode
5. Document all permissions

**Effort estimate:** 8-16 hours for development and testing

### Security Audit

If publishing to OpenStore or enterprise deployment requires security audit:

- Document all permissions granted
- Justify each policy group
- Provide threat model
- Consider custom profile if auditor recommends

This is not required for community/personal use.

## Alternatives Considered

### Alternative 1: Custom AppArmor Profile from Start
- **Pros**: Maximum security, best practice
- **Cons**: Time-consuming, delays Sprint 002, requires expertise
- **Rejected**: Over-engineering for current needs; can revisit later

### Alternative 2: Request "unconfined" Exception
- **Pros**: No code changes needed
- **Cons**: Won't be granted; unprofessional; insecure
- **Rejected**: Not viable, violates platform standards

### Alternative 3: Different Technology Stack
- **Pros**: Might avoid AppArmor complexity
- **Cons**: Requires complete rewrite; Bluetooth still needs permissions
- **Rejected**: Not practical; issue is policy, not technology

## References

- [UBports AppArmor Policy Groups Documentation](https://docs.ubports.com/en/latest/appdev/platform/apparmor.html)
- [Ubuntu AppArmor Wiki](https://wiki.ubuntu.com/AppArmor)
- [OpenStore App Submission Guidelines](https://open-store.io/)
- AppArmor Analysis: `docs/sprint-002/apparmor-analysis.md`
- Ian L.'s ubtd-20.04: `github.com/floop2002/ubtd-20.04`

## Decision Log

- **2025-12-18**: Identified issue during Sprint 001 completion
- **2025-12-18**: Analyzed alternatives, researched best practices
- **2025-12-18**: Decision made to use `bluetooth` policy group
- **2025-12-18**: ADR-002 created
- **Sprint 002**: Implement and validate on hardware

## Review and Updates

This decision will be reviewed after Sprint 002 hardware testing. If AppArmor denials or permission issues occur, we'll evaluate whether a custom profile is necessary for Sprint 003.

**Next review**: End of Sprint 002 (after hardware testing results)
