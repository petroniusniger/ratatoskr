# Sprint #002: test result
(pan, 19/12/2025)

## Compilation / auto-review on host arch.

- Build successful
- Auto-review produces the following errors (but they were expected):

  ```
   - security:policy_groups_safe:ratatoskr:bluetooth
	  (NEEDS REVIEW) reserved policy group 'bluetooth': vetted applications only
   - security:policy_groups_safe:shareplugin:bluetooth
	  (NEEDS REVIEW) reserved policy group 'bluetooth': vetted applications only
  ```
  
## Start application inside emulator

- MainWindow displays as expected: [screenshot](./MainWindow_EMU-002.jpg)
- NearbyDevices screen displays when the top-right icon is clicked: [screenshot](./NearbyDevice_EMU-002.jpg)

## Compilation / auto-review for target arch. (`arm64`)

- Build successful
- Auto-review produces the same errors as above (just as expected)

