# 'ratatoskr' SharePlugin test report #2
(pan, v0, 22/01/2026)


Test performed using the following package: `ratatoskr.philipa_260121164817_arm64.click` 
(result of Sprint #006).

## Test steps and corresponding log entries

- connect phone to laptop over BT through 'system settings' app:

  Result: laptop connected, confirmed by status icon on both sides.

- launch 'ratatoskr' main app to have it start OBEX daemon:

  ```
  Jan 22 10:13:09 hatshepsut systemd[2016]: Started lomiri-app-launch--application-click--ratatoskr.philipa_ratatoskr_260121164817--.service.
  ```

- stop 'ratatoskr' main app as it should no longer be needed:

  ```
  Jan 22 10:13:18 hatshepsut lomiri[3178]: [2026-01-22:10:13:18.036] ApplicationManager::onProcessStopped reports stop of appId= "ratatoskr.philipa_ratatoskr" which AppMan is not managing, ignoring the event
  ```

- open "Contacts" app:

  ```
  Jan 22 10:13:54 hatshepsut systemd[2016]: Started lomiri-app-launch--application-legacy--address-book-app--.service.
  ```

- open a random contact in the list
- click on its "Share" icon 

  Result: the "Share to" selector appears with the following apps listed:
  
    - Messaging
    - File Manager
    - Dekko 2
    - ratatoskr.philipa_shareplu
    
- click on "ratatoskr.philipa_shareplu":

  ```
  Jan 22 10:13:59 hatshepsut com.lomiri.content.dbus.Service[3245]: QMap<QString, QString> {anonymous}::info_for_app_id(QString) Failed to create Application for "ratatoskr.philipa_shareplugin_260121164817"
  [...]
  Jan 22 10:14:03 hatshepsut content-hub-ser[3245]: Unable to start app 'ratatoskr.philipa_shareplugin_260121164817': Application is not meant to be displayed
  ```

  Result:
  
    - a pop-up appears for a fraction of a second (too fast to read what it 
      says)
    - the phone restarts (soft restart)
    - the laptop doesn't seem to detect any BT transfer attempt
    
## Full 'journal' content for test duration

See: `docs/sprint-006/journal_cropped-20260122.txt`
