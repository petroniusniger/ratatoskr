# Sprint #007: 'ratatoskr' SharePlugin test report
(pan, v0, 22/01/2026)


Test performed using the following package: `ratatoskr.philipa_260122124335_arm64.click` 
(result of Sprint #007).

## Test steps and corresponding log entries

- installed latest package `ratatoskr.philipa_260122124335_arm64.click`

  Result:
  
  - installation successful
  - a new app called 'Bluetooth' appears in the app drawer
  - the new app 'Bluetooth' has no icon

- connect phone to laptop over BT through 'system settings' app:

  ```
  Jan 22 16:32:30 hatshepsut systemd[2016]: Started lomiri-app-launch--application-legacy--lomiri-system-settings--.service.
  ```

  Result: laptop connected, confirmed by status icon on both sides.

- stop OBEX daemon as it is not needed:

  Result: `obexd` stopped.
  
- open "Contacts" app:

  ```
  Jan 22 16:34:20 hatshepsut systemd[2016]: Started lomiri-app-launch--application-legacy--address-book-app--.service.
  ```

- open a random contact in the list
- click on its "Share" icon 

  Result: the "Share to" selector appears with the following apps listed:
  
    - Messaging
    - File Manager
    - Dekko 2
    - Bluetooth
    
- click on "Bluetooth":

  ```
  Jan 22 16:34:48 hatshepsut content-hub-ser[3245]: Unable to stop app 'ratatoskr.philipa_shareplugin_260121164817': Invalid app ID: ratatoskr.philipa_shareplugin_260121164817
  Jan 22 16:34:48 hatshepsut address-book-app[5174]: qml: 'caller' is DEPRECATED. It has no effect.
  Jan 22 16:34:48 hatshepsut address-book-app[5174]: qml: No active transfer
  Jan 22 16:34:48 hatshepsut systemd[2016]: Started lomiri-app-launch--application-click--ratatoskr.philipa_shareplugin_260122124335--.service.
  ```

  Result:
  
    - a new app starts that takes the whole screen
    - the new app background is completely black with the "Bluetooth" word
      written in the middle of the screen
    - a rotating blue circle animation denotes waiting for something, but
      nothing more happens
    - the laptop doesn't seem to detect any BT transfer attempt
    
  Note:
  
    - the laptop's Bluetooth adaptor MAC address is `94:E6:F7:22:D6:56` (public)

- I eventually stopped the SharePlugin app manually:

  ```
  Jan 22 16:36:11 hatshepsut systemd[2016]: Stopping lomiri-app-launch--application-click--ratatoskr.philipa_shareplugin_260122124335--.service...
  Jan 22 16:36:11 hatshepsut systemd[2016]: lomiri-app-launch--application-click--ratatoskr.philipa_shareplugin_260122124335--.service: Succeeded.
  Jan 22 16:36:11 hatshepsut systemd[2016]: Stopped lomiri-app-launch--application-click--ratatoskr.philipa_shareplugin_260122124335--.service.
  ```

- the crash leading to a restart of the phone no longer occurs
    
## Full 'journal' content for test duration

See: `docs/sprint-007/journal_cropped-20260122-2.txt`
