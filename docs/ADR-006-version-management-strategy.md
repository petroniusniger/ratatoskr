# ADR-006: Version Management Strategy

**Date:** 2026-02-11  
**Status:** Accepted  
**Author:** Philippe Andersson + Copilot CLI

## Context

Ratatoskr consists of two executables (main app and SharePlugin) that need to log their version at startup for debugging and support purposes. We need a consistent strategy for version management that:

1. Works for both executables
2. Is maintained in a single authoritative source
3. Is automatically generated from git tags
4. Is accessible at runtime for logging

## Current State

The root `CMakeLists.txt` already implements version generation:
- Uses `git describe` to extract version from git tags
- Defines `BUILD_VERSION` as a compiler flag (`-DBUILD_VERSION="${BUILD_VERSION}"`)
- Tagged commits use the tag as version (e.g., `0.0.1`)
- Untagged commits use `{last-tag}.{timestamp}` format

However, this `BUILD_VERSION` define is only available to the main app sources, not to the SharePlugin subdirectory.

## Decision

We will use the existing CMake-based version generation with the following strategy:

### 1. Single Source of Truth
- Git tags remain the authoritative version source
- Root `CMakeLists.txt` continues to generate `BUILD_VERSION`
- No manual version strings in source files

### 2. SharePlugin Access to Version
The `BUILD_VERSION` preprocessor define will be made available to the SharePlugin by:
- The root CMakeLists.txt already defines it globally via `add_definitions(-DBUILD_VERSION="${BUILD_VERSION}")`
- This makes it available to all targets, including the SharePlugin subdirectory
- No additional CMake changes needed

### 3. Runtime Logging
Both executables will log their version at startup using:
```cpp
qDebug() << "Application Name" << BUILD_VERSION << "starting...";
```

The `BUILD_VERSION` macro will be available as a string literal at compile time.

### 4. Version Format
- Release builds (tagged commits): `X.Y.Z` (e.g., `0.0.1`)
- Development builds: `X.Y.Z.YYMMDDHHMMSS` (e.g., `0.0.1.260211153000`)

## Implementation

1. Main app (`ratatoskr/main.cpp`):
   - Add `qDebug()` statement immediately after `QGuiApplication` initialization
   - Format: `"Ratatoskr" BUILD_VERSION "starting..."`

2. SharePlugin (`ratatoskr/shareplugin/main.cpp`):
   - Add `qDebug()` statement immediately after `QGuiApplication` initialization
   - Format: `"Ratatoskr SharePlugin" BUILD_VERSION "starting..."`

3. Version generation (`ratatoskr/get-version.sh`):
   - Script runs on host (where git is available) before Docker build
   - Generates `VERSION.txt` with format: `vX.Y.Z.YYMMDDHHMMSS`
   - CMakeLists.txt reads VERSION.txt and exposes as BUILD_VERSION macro

4. Build process (`ratatoskr/build.sh`):
   - Wrapper script that calls `get-version.sh` before invoking clickable
   - Ensures VERSION.txt is always regenerated with current timestamp
   - Usage: `./build.sh build` or `./build.sh build --arch arm64`

**Important**: Always use `./build.sh` instead of calling `clickable` directly to ensure version timestamps are current.

## Consequences

### Positive
- Single source of truth (git tags)
- Automatic version generation
- No manual version maintenance in code
- Consistent versioning across both executables
- Easy to identify build in logs

### Negative
- Requires git repository for builds (already a requirement)
- Version only known at compile time (not modifiable at runtime)
- Timestamp-based dev versions not semantic versioning compliant
- **Developers must use `./build.sh` wrapper instead of calling `clickable` directly**
- VERSION.txt must be regenerated before each build for accurate timestamps

### Neutral
- Developers must use git tags for releases
- Version information appears in logs (requires log access for support)

## References
- Root CMakeLists.txt lines 45-63 (version generation)
- Sprint #010 scope: "App and SharePlugin log their version at startup"
