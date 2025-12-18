# Sprint 002 Scope

**Sprint**: 002  
**Start Date**: 2025-12-18  
**Target Version**: 0.0.2  
**Goal**: Fix AppArmor security policy and establish working baseline on real hardware

## Sprint Objectives

The primary objectives of Sprint 002 are to:
1. **Fix critical AppArmor security issue** blocking OpenStore publication
2. **Test on real Ubuntu Touch hardware** to validate functionality
3. **Document baseline functionality** before adding new features
4. **Identify any platform-specific issues** with Bluetooth on UT 20.04

## Priority #1: AppArmor Security Fix

### Critical Issue
The `unconfined` AppArmor template blocks OpenStore publication and represents a security vulnerability.

### Solution
Replace with `bluetooth` policy group - the standard approach for Bluetooth apps on Ubuntu Touch.

### Deliverables
- [ ] Update `ratatoskr.apparmor` to use `bluetooth` policy group
- [ ] Update `shareplugin/shareplugin.apparmor` to use `bluetooth` policy group
- [ ] Remove `"template": "unconfined"` from both files
- [ ] Verify build passes automated review
- [ ] Document the change in ADR-002

## Priority #2: Hardware Testing

### Objective
Validate that the app works on real Ubuntu Touch 20.04 hardware with actual Bluetooth capabilities.

### Test Scenarios

#### Bluetooth Adapter Testing
- [ ] App launches on real device
- [ ] Bluetooth adapter is detected
- [ ] Device can be made discoverable
- [ ] No AppArmor denials in system logs

#### File Receiving (Main App)
- [ ] OBEX agent registers with bluez-obexd
- [ ] Can receive file from PC via Bluetooth
- [ ] Can receive file from Android phone
- [ ] Received files saved to correct location
- [ ] Transfer progress displayed in UI
- [ ] Transfer completion notification works

#### File Sending (SharePlugin)
- [ ] Can discover nearby Bluetooth devices
- [ ] Can select target device
- [ ] Can send file via OBEX OPP
- [ ] Transfer progress displayed
- [ ] Success/error feedback works
- [ ] ContentHub integration functional

#### Error Handling
- [ ] Graceful handling if Bluetooth is disabled
- [ ] Appropriate errors for pairing failures
- [ ] Clear feedback for transfer failures

### Deliverables
- [ ] Test execution on real UT 20.04 device
- [ ] Document test results in `sprint-002/testing-report.md`
- [ ] Log any AppArmor denials or permission issues
- [ ] Capture screenshots of working functionality
- [ ] Document any bugs discovered

## Priority #3: QML Component Deprecation Fixes

### Issue
The UI uses deprecated `UbuntuShape` component causing QML warnings.

### Solution
Update to Lomiri 1.3 components (if time permits in Sprint 002, otherwise defer to Sprint 003).

### Deliverables (Optional - if time permits)
- [ ] Identify all uses of deprecated components in Main.qml
- [ ] Replace `UbuntuShape` with `Lomiri.Components.Shape`
- [ ] Test UI rendering after changes
- [ ] Verify no visual regressions

**Note:** This is lower priority than AppArmor fix and hardware testing. If time is limited, defer to Sprint 003.

## Priority #4: Documentation Updates

### Deliverables
- [ ] Create ADR-002 documenting AppArmor policy decision
- [ ] Update architecture.md with security policy details
- [ ] Document testing methodology and results
- [ ] Update README with hardware testing notes
- [ ] Create Sprint 002 completion summary

## Out of Scope

The following are explicitly **not** included in Sprint 002:

- ❌ Contacts API integration
- ❌ VCF export functionality
- ❌ Car kit specific features
- ❌ UI/UX redesign
- ❌ Device discovery improvements
- ❌ Custom AppArmor profile development
- ❌ Performance optimizations
- ❌ Translations/internationalization
- ❌ New features

**Focus:** Fix critical issues, validate baseline, prepare for feature development in Sprint 003+

## Success Criteria

Sprint 002 is considered successful when:

1. ✅ **AppArmor fix implemented** and build passes automated review
2. ✅ **App tested on real hardware** with documented results
3. ✅ **File receiving works** on actual device
4. ✅ **File sending works** on actual device
5. ✅ **No critical bugs** blocking basic functionality
6. ✅ **Documentation updated** with findings

## Technical Tasks

### Task 1: AppArmor Configuration Update
**Estimated Effort:** 30 minutes

**Files to modify:**
- `ratatoskr/ratatoskr.apparmor`
- `ratatoskr/shareplugin/shareplugin.apparmor`

**Changes:**
```json
{
    "policy_groups": [
        "bluetooth",      // ADD THIS
        "networking",
        "content_exchange_source"  // or "content_exchange" for shareplugin
    ],
    "policy_version": 20.04
    // REMOVE: "template": "unconfined"
}
```

**Validation:**
```bash
cd ratatoskr/
clickable build
# Should complete without "unconfined" errors
```

### Task 2: Real Hardware Deployment
**Estimated Effort:** 1-2 hours (including setup)

**Prerequisites:**
- Ubuntu Touch 20.04 device available
- Device connected via USB or network
- Developer mode enabled on device

**Steps:**
```bash
cd ratatoskr/
clickable install  # Install to connected device
clickable logs     # Monitor logs during testing
```

**Testing:**
1. Launch app from device
2. Check Bluetooth settings
3. Attempt file receive from PC
4. Test file send to PC
5. Monitor system logs for issues

### Task 3: Documentation
**Estimated Effort:** 1-2 hours

**Documents to create:**
- `docs/ADR-002-apparmor-bluetooth-policy.md`
- `docs/sprint-002/testing-report.md`
- `docs/sprint-002/completion-summary.md`

**Documents to update:**
- `docs/architecture.md` - Add AppArmor policy section
- `ratatoskr/README.md` - Update with testing status

## Risk Assessment

### Risk 1: AppArmor Policy Insufficient
**Probability:** Low  
**Impact:** High  
**Mitigation:** 
- "bluetooth" is standard policy for BT apps
- Can monitor logs for denials
- Fallback: Create custom profile in Sprint 003

### Risk 2: Hardware-Specific Bluetooth Issues
**Probability:** Medium  
**Impact:** High  
**Mitigation:**
- Test on multiple devices if possible
- Document device-specific quirks
- Engage UBports community if needed

### Risk 3: OBEX Daemon Not Running
**Probability:** Low  
**Impact:** High  
**Mitigation:**
- Verify bluez-obexd service status
- Check system configuration
- Document service dependencies

### Risk 4: No Access to Real Hardware
**Probability:** Low (assuming you have device)  
**Impact:** High  
**Mitigation:**
- Continue with emulator testing
- Plan hardware testing for Sprint 003
- Focus on AppArmor fix only

## Dependencies

### Required
- Ubuntu Touch 20.04 device (for hardware testing)
- USB debugging enabled
- Bluetooth hardware functioning
- Test PC or Android device for transfers

### Optional
- Multiple UT devices for broader testing
- Various Bluetooth devices for compatibility testing

## Timeline Estimate

**Total Estimated Time:** 4-6 hours

- AppArmor fix: 30 minutes
- Build verification: 15 minutes
- Hardware setup: 30 minutes
- Functional testing: 2-3 hours
- Documentation: 1-2 hours
- Buffer for issues: 1 hour

**Recommended approach:** Complete over 1-2 days to allow for thorough testing

## Definition of Done

Each deliverable is considered "done" when:
- Code changes committed with proper headers
- Build completes successfully
- Functionality verified (on hardware if applicable)
- Documentation updated
- Git commit with descriptive message

## Sprint Review Preparation

At sprint completion, prepare to demonstrate:
1. Clean build without AppArmor errors
2. App running on real hardware (video/screenshots)
3. Successful file transfer (receive and send)
4. Documentation of any issues found
5. Recommendations for Sprint 003

## Next Sprint Preview

Based on Sprint 002 results, Sprint 003 may focus on:
- Fixing any critical bugs discovered
- Contacts API integration
- VCF export implementation
- Car kit compatibility testing
- UI improvements based on testing feedback
- Custom AppArmor profile (if needed)

The exact scope will be determined after Sprint 002 completion and hardware testing results.

## Notes

- **Priority is AppArmor fix** - This unblocks everything else
- **Hardware testing is crucial** - Need to validate assumptions
- **Don't rush** - Thorough testing now saves time later
- **Document everything** - Findings will inform Sprint 003+

## References

- Sprint 001 completion: `docs/sprint-001/completion-summary.md`
- AppArmor analysis: `docs/sprint-002/apparmor-analysis.md`
- Architecture docs: `docs/architecture.md`
- ADR-001: `docs/ADR-001-bluetooth-architecture-strategy.md`
