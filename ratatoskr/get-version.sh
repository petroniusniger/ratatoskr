#!/bin/bash
# Generate version file for CMake
# This runs on the host before Docker build, where git is available

cd "$(dirname "$0")/.." || exit 1

# Try to get exact tag match
VERSION=$(git describe --tags --abbrev=0 --exact-match 2>/dev/null)

if [ -z "$VERSION" ]; then
  # Not on a tag, get last tag and append timestamp
  LAST_TAG=$(git describe --tags --abbrev=0 2>/dev/null || echo "v0.0.0")
  TIMESTAMP=$(date -u +"%y%m%d%H%M%S")
  VERSION="${LAST_TAG}.${TIMESTAMP}"
fi

# Write to file that CMake will read
echo "$VERSION" > ratatoskr/VERSION.txt
echo "Generated VERSION.txt: $VERSION"

# Touch CMakeLists.txt to force reconfiguration
touch ratatoskr/CMakeLists.txt
