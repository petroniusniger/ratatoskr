# Ratatoskr Installation Guide

## Standard Installation

The `.click` packages can be installed normally via:

- OpenStore (requires manual review due to `bluetooth` policy group)
- Manual installation: `pkcon install-local ratatoskr.philipa_*.click`

## AppArmor Override (Required for File Transfer)

Due to Ubuntu Touch platform limitations, the standard `bluetooth` policy group does not include D-Bus receive permissions needed for OBEX Agent callbacks. File transfers will **not work** without applying the AppArmor override.

### Manual Override Installation

1. Install the `.click` package normally
2. Copy `ratatoskr.apparmor.local` to your device
3. Open a terminal on the device and run:

```bash
# Find your app's profile name
PROFILE=$(ls /var/lib/apparmor/clicks/ | grep ratatoskr.philipa_ratatoskr)

# Append the override rules
sudo cat ratatoskr.apparmor.local >> /var/lib/apparmor/clicks/$PROFILE

# Reload the profile
sudo apparmor_parser -r /var/lib/apparmor/clicks/$PROFILE
```

4. Verify the override was applied:
```bash
sudo cat /var/lib/apparmor/clicks/$PROFILE | grep "dbus (receive)"
```

You should see the D-Bus receive rules at the end of the profile.

### Testing

After applying the override:

1. Ensure Bluetooth is enabled and paired with a device
2. Start the Ratatoskr app
3. Send a file from your paired device
4. The app should prompt for authorization and save the file

Check logs if issues persist:
```bash
journalctl -f | grep -E "(ratatoskr|obexd|apparmor)"
```

## Known Limitations

- **OpenStore Distribution**: The app requires manual review for OpenStore due to:
  - Use of reserved `bluetooth` policy group
  - Need for manual AppArmor override

- **Updates**: The AppArmor override must be reapplied after each app update

- **Platform Dependency**: This workaround is necessary until Ubuntu Touch adds:
  - D-Bus receive permissions to the `bluetooth` policy group, OR
  - Support for custom D-Bus rules in JSON apparmor manifests, OR
  - A new policy group (e.g., `bluetooth-agent`) for OBEX implementations

## For Developers

See `docs/ADR-004-apparmor-dbus-receive-permissions.md` for technical details about this limitation and the architectural decision.

## Support

If you encounter issues:

1. Check that OBEX daemon is running: `ps aux | grep obexd`
2. Verify AppArmor profile is loaded: `sudo aa-status | grep ratatoskr`
3. Check for AppArmor denials: `sudo journalctl | grep apparmor | grep DENIED | grep ratatoskr`
4. Report issues at: [project repository URL]
