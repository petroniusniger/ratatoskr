# Ratatoskr

Bluetooth File Transfer for Ubuntu Touch 20.04

## Description

*Ratatoskr* is a Bluetooth file transfer application for Ubuntu Touch, with a 
primary focus on sending contacts (VCF cards) to Bluetooth car kits. It also 
supports bi-directional file transfer with PCs.

The app is named after the mischievous, gossipy squirrel in Norse mythology 
who inhabits the world tree, Yggdrasil and carries slanderous messages between 
the eagles perched atop it and the dragon Níðhöggr who dwells beneath  
the roots of the tree. He seemed to me like a worthy companion to Harald 
Bluetooth (provided he doesn't distort your files too much) ;-)

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

The description of the Norse mythological character is borrowed from its
[Wikipedia page](https://en.wikipedia.org/wiki/Ratatoskr).

Original *Ratatoskr* icon built by *ChatGPT*.

## Usage guidelines

### Sending files / vCards

- pair with remote device and connect to it through System Settings
- open appropriate application and access desired data (e.g. File Manager for
  regular files or Contacts for vCards)
- left-swipe on data item, tap Share icon
- on "Open With" screen, select BT Plugin
- tap desired target in device list (paired devices are usually displayed 
  closed to the top)
- the target device may prompt to accept the transfer
- a progress bar will appear during transfer and a green button will take its
  place when the transfer is complete -- tap it to close the BT Plugin
  
**Please note:** if you are surrounded by a large number of BT devices, there
may be a lag (of up to several seconds) between tapping the target device in 
the list and the actual start of the file transfer.

### Receiving files

- pair with remote device and connect to it through System Settings
- start Ratatoskr app
- on remote device, share file to phone over BT
- the file appears on the main Ratatoskr screen (with its name and a small 
  thumbnail)
- left-swipe on the file, tap Download icon
- on "Open With" screen, select File Manager
- navigate to desired folder, validate with 'V' button
- the incoming file is copied to the selected folder

Once done, the incoming file can be deleted from the Ratatoskr cache:

- return to Ratatoskr main page
- right-swipe on the file, tap Delete icon

## Build Instructions

```bash
cd ratatoskr/
./build.sh build --arch=arm64
```

## Run in Desktop Mode

```bash
./build.sh desktop
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
