# Sprint #010: scope
(pan, v0, 11/02/2026)

## Context

Sprint #010 is meant to be the last one before publishing the first public
release of the application on the OpenStore. It's scope will be to resolve a 
few minor issues and to clean up the code where needed.

## Methodology

Issues will be tackled and resolved one by one. Code changes will be committed
and tested before jumping to the next one.

## Scope

### Icon for SharePlugin

The SharePlugin lacks an icon. There are 2 places where it should be visible:

- in the App Drawer: [screenshot](./screenshot20260211_135522501.png)
- on the SharePlugin selection screen, after tapping the Share button: 
  [screenshot](./screenshot20260211_135538457.png)
  
Icon to use is `ratatoskr/shareplugin/icon.svg`

### App and SharePlugin log their version at startup

Both the main 'Ratatoskr' app and the SharePlugin should emit a log message
containing their name and version number when they start.

An ADR should be created to determine the best place to store and maintain
that version string.

### Warning pop-up if SharePlugin is started directly from App Drawer

As a general rule, SharePlugins should not appear at all in the App Drawer.
The current implementation was selected as a shortcut to achieve the desired
functionalities. But this means that a user can start the SharePlugin
directly, without going through the worflow of selecting a media and tapping 
"Share".

A pop-up displaying a warning message should be added when the SharePlugin is
directly started from the App Drawer, to the effect that the "Share" workflow
should be used instead. 

### Code cleanup in App if needed

Check the code for the main 'Ratatoskr' app and determine whether the following
objects are really used at runtime:

- BluetoothManager
- DeviceDiscovery
- OBEXTransferService
  
Remove the related code if they are not needed.

### Comment code if needed

Add comments in the code where its complexity makes it worthwhile.
