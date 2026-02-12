#!/bin/bash
# Wrapper script for clickable build that ensures VERSION.txt is up-to-date
# Usage: ./build.sh [clickable arguments]
#
# This script:
# 1. Regenerates VERSION.txt with current timestamp
# 2. Runs clickable with the provided arguments

set -e

cd "$(dirname "$0")"

# Regenerate version
./get-version.sh

# Run clickable with all arguments passed to this script
clickable "$@"
