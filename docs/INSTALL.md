# Ratatoskr Installation Guide

## Standard Installation

The `.click` packages can be installed normally via:

- OpenStore (requires manual review due to `bluetooth` policy group)
- Manual installation:
  - copy `.click` package to phone, place it below `Downloads/`
  - start File Manager, open `Downloads/` folder, tap package file
  - in "Open With", select OpenStore app
  - a warning will appear about installing untrusted apps: accept and continue
- Manual installation (CLI alternative): `pkcon install-local ratatoskr.philipa_*.click`

## Known Limitations

- **OpenStore Distribution**: The app requires manual review for OpenStore due 
  to:
  - Use of reserved `bluetooth` policy group by the SharePlugin
  - Use of unconfined AppArmor profile by the main app

- **Platform Dependency**: This use of an unconfined AppArmor profile is 
  necessary until Ubuntu Touch adds:
  - D-Bus receive permissions to the `bluetooth` policy group, OR
  - Support for custom D-Bus rules in JSON apparmor manifests, OR
  - A new policy group (e.g., `bluetooth-agent`) for OBEX implementations

## For Developers

See `docs/ADR-004-apparmor-dbus-receive-permissions.md` for technical details 
about this limitation and the architectural decision.

## Support

If you encounter issues:

- with the main app:
  - Check that OBEX daemon is running: `ps aux | grep obexd`
- with the SharePlugin:
  - Check for AppArmor denials: `sudo journalctl | grep apparmor | grep DENIED | grep ratatoskr`

Report issues at: [project repository URL]
