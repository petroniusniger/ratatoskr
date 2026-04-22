# Ratatoskr

Bluetooth File Transfer for Ubuntu Touch 20.04

## Description

*Ratatoskr* is a Bluetooth file transfer application for Ubuntu Touch, with an 
added focus on sending contacts (VCF cards) to Bluetooth car kits. It also 
supports bi-directional file transfer with PCs or other phones.

The app is named after the mischievous, gossipy squirrel in Norse mythology 
who inhabits the world tree, Yggdrasil, and carries slanderous messages between 
the eagles perched atop it and the dragon Níðhöggr who dwells beneath 
the roots of the tree. He seemed to me like a worthy companion to Harald 
Bluetooth (hoping he doesn't distort your files too much) ;-)

Install it from the OpenStore:  
[![OpenStore](https://next.open-store.io/badges/en_US.png)](https://next.open-store.io/app/ratatoskr.petroniusniger/)

## Features

- **Send files** via Bluetooth using OBEX Object Push Profile (OPP)
- **Receive files** from other Bluetooth devices
- **ContentHub integration** for sharing files from other apps
- **VCF support** optimized for car kit contact synchronization

## Attribution

This application is based on 'ubtd' (Bluetooth File Transfer) originally developed by:

- **Michael Zanetti** <michael.zanetti@canonical.com> - Original development
- **Ian L.** (floop2/fourloop2002) - Ubuntu Touch 16.04 and 20.04 ports

Revived and extended as 'Ratatoskr' by Philippe Andersson for Les Ateliers du 
Héron, 2025, with the help of the Copilot CLI coding agent.

Original *Ratatoskr* icon built with the help of *ChatGPT*.

The description of the Norse mythological character is borrowed from its
[Wikipedia page](https://en.wikipedia.org/wiki/Ratatoskr).

## Usage guidelines

### Sending files / vCards

- pair with remote device and connect to it through System Settings
- open appropriate application and access desired data (e.g. File Manager for
  regular files or Contacts for vCards)
- left-swipe on data item, tap Share icon
- on "Open With" screen, select 'BT Plugin'
- tap desired target in device list (paired devices are usually displayed 
  close to the top)
- the target device may prompt to accept the transfer
- a progress bar will appear during transfer and a green button will take its
  place when the transfer is complete -- tap it to close the BT Plugin
  
**Please note:** if you are surrounded by a large number of BT devices, there
may be a lag (of up to several seconds) between tapping the target device in 
the list and the actual start of the file transfer -- please be patient.

### Receiving files

- pair with remote device and connect to it through System Settings
- start Ratatoskr app
- on remote device, share file to phone over BT
- a pop-up appears on the phone, asking you to accept or reject the transfer
- if you accept, the file appears on the main Ratatoskr screen (with its name 
  and icon or a small thumbnail if applicable)
- left-swipe on the file, tap Download icon
- on the "Open With" screen, select File Manager
- navigate to desired folder, validate with 'V' button
- the incoming file is copied to the selected folder

Once done, the incoming file can be deleted from the Ratatoskr cache:

- return to Ratatoskr main page
- right-swipe on the file, tap Delete icon, confirm

## Build Instructions

```bash
cd ratatoskr/
clickable build --arch=arm64
```

(adapt target architecture as needed).

## Run in Desktop Mode

```bash
clickable desktop
```

## Technical Details

- Built with Qt 5.12.9 and QML
- Uses Qt Bluetooth for OBEX file sending
- Uses D-Bus interface to bluez-obexd for receiving files
- Requires Ubuntu Touch 20.04 framework

## Support

Report issues at: [GitHub 'ratatoskr' project](https://github.com/petroniusniger/ratatoskr)

## Contributing to the project

Please follow the workflow described below:

- [Create an issue](https://github.com/petroniusniger/ratatoskr/issues/new/choose)
  that describes your problem or new feature (if the scope is obvious, filling 
  the title field is enough)
- from the issue page, create a branch (in the right-side menu)
- clone project, checkout branch, edit, git add, git commit, git push...
- when your modifications are ready, create a pull request for your branch
  - in the PR description field, type "closes #<issue-nr\>"
- I will be notified, review your changes and merge them to 'main'

## License

Copyright (C) Original work by Michael Zanetti, Canonical

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
