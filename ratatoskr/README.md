# Ratatoskr

Bluetooth File Transfer for Ubuntu Touch 20.04

## Description

Ratatoskr is a Bluetooth file transfer application for Ubuntu Touch, with a primary focus on sending contacts (VCF cards) to Bluetooth car kits. It also supports bi-directional file transfer with PCs.

## Features

- **Send files** via Bluetooth using OBEX Object Push Profile (OPP)
- **Receive files** from other Bluetooth devices
- **ContentHub integration** for sharing files from other apps
- **VCF support** optimized for car kit contact synchronization

## Attribution

This application is based on 'ubtd' (Bluetooth File Transfer) originally developed by:

- **Michael Zanetti** <michael.zanetti@canonical.com> - Original development
- **Ian L.** (floop2/fourloop2002) - Ubuntu Touch 16.04 and 20.04 ports

Adapted and extended as 'Ratatoskr' by Philippe Andersson for Les Ateliers du Heron, 2025.

## Build Instructions

```bash
cd ratatoskr/
clickable build --arch=arm64
```

## Run in Desktop Mode

```bash
clickable desktop
```

## Technical Details

- Built with Qt 5.12.9 and QML
- Uses Qt Bluetooth for OBEX file sending
- Uses D-Bus interface to bluez-obexd for receiving files
- Requires Ubuntu Touch 20.04 framework

## License

Copyright (C) 2025  Philippe Andersson

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License version 3, as published by the
Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranties of MERCHANTABILITY, SATISFACTORY
QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.

Original work by Michael Zanetti and Ian L. also under GNU GPL v3.
