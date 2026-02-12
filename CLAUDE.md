# Tech Stack

Note: the compiler and related libraries are installed inside a docker
container managed by 'clickable'

- Language: GNU C++ 9.4.0
- Libraries: 
  - Qt/QML 5.12.9
- Build tool: 
  - for host arch. (emulator): `./build.sh build` in `ratatoskr/` subfolder
  - for target arch. (aarch64): `./build.sh build --arch arm64` in `ratatoskr/` subfolder
  - Note: Use `build.sh` wrapper instead of calling `clickable` directly to ensure VERSION.txt is regenerated
- Emulator: clickable-ut 8.6.0 + docker 28.5.1

# Project Structure

- `docs/`: technical and project management documentation
- `ratatoskr/`: project sources, header files, QML files, etc.
- `templates/`: source files templates

# Code Style

- tab value = 2 spaces
- use source file header from `templates/` folder
- update last modification date and version history in header each time a 
  source file is modified

# Project management

- use ADR
- store each ADR in the `docs/` folder 
- keep an index of existing ADR in this file
- work in sprint
- we'll discuss the scope of each sprint before it begins

## ADR Index

- [ADR-001: Bluetooth File Transfer Architecture Strategy](docs/ADR-001-bluetooth-architecture-strategy.md) - 2025-12-18
- [ADR-002: AppArmor Security Policy for Bluetooth Access](docs/ADR-002-apparmor-bluetooth-policy.md) - 2025-12-18
- [ADR-003: OBEX D-Bus Service Discovery and Agent Registration](docs/ADR-003-obex-dbus-service-discovery.md) - 2026-01-15
- [ADR-004: AppArmor D-Bus Receive Permissions for OBEX Agent](docs/ADR-004-apparmor-dbus-receive-permissions.md) - 2026-01-20
- [ADR-005: SharePlugin NoDisplay Issue and ContentHub Integration](docs/ADR-005-shareplugin-nodisplay-issue.md) - 2026-01-22
- [ADR-006: Version Management Strategy](docs/ADR-006-version-management-strategy.md) - 2026-02-11

# Versioning Strategy

- use git
- project is a local repository
- use semantic versioning
- during project inception, target version is called 0.0.1
- development for each sprint will occur in a branch called `sprint-XXX` where 
  `XXX` represents the sprint sequence number
- DO NOT merge in `master` branch -- I will take care of it at the end of each
  sprint
  

