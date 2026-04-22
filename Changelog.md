# Changelog for project 'ratatoskr'
(pan, 22/04/2026)


## v0.1.3 (*upcoming*)

- Added recommended Github workflow for project contributors to the README 
  document.
- Added OpenStore "badge" to the README document.
- Added German translation contributed by buchnerh (Github 
  [issue #31](https://github.com/petroniusniger/ratatoskr/issues/31)).

## v0.1.2 (Apr. 16, 2026)

- Minor updates to test procedure.
- Added Dutch translation contributed by Heimen Stoffels (Github 
  [issue #29](https://github.com/petroniusniger/ratatoskr/issues/29)).
- Removed './docs/Roadmap.md' as its content and function have now been moved
  to Github issues, milestones and releases.
- Fixed buildchain to correctly populate POT file. Replaced `intltool-extract` 
  with modern two-pass `xgettext` approach. Created `po/POTFILES.in` with 
  explicit list of all translatable source files. Provided first translations
  (French, Italian). Updated test procedure accordingly. Wrapped missing strings 
  in 'i18n.tr()' calls, re-generated POT file and merged new strings in French 
  PO file. See Github 
  [issue #20](https://github.com/petroniusniger/ratatoskr/issues/20).
- Renamed SharePlugin icon from `icon.svg` to `sp_BT_logo.svg`. Handled related 
  impacts. See Github [issue #23](https://github.com/petroniusniger/ratatoskr/issues/23).
- Got rid of `build.sh` and `get-version.sh` helper scripts and returned to a 
  standard build process (Github 
  [issue #24](https://github.com/petroniusniger/ratatoskr/issues/24)).
  Updated test procedure accordingly.
- Removed Copilot CLI from all authorship assignments. Fixed copyright 
  assignment in `./ratatoskr/main.cpp`. Created separate header templates for
  files inherited from 'ubtd' project and new ones, aligned all source files. 
  Added comment in headers about use of coding agent. Updated `./CLAUDE.md` and 
  `./.github/copilot-instructions.md` to specify the use of a `Co-developed-by:` 
  git trailer for all commits performed by Copilot. See 
  Github [issue #22](https://github.com/petroniusniger/ratatoskr/issues/22).

## v0.1.1 (Mar. 17, 2026)

- Updated test procedure as a result of its first formal execution.
- Added confirmation dialog before deleting cached files in main app 
  (Github [issue #5](https://github.com/petroniusniger/ratatoskr/issues/5)).
  Updated test procedure accordingly.
- Reverted the SharePlugin to an unconfined template as the notion that it
  worked with an enforcing AA profile turned out to be wrong. Updated Github
  [issue #7](https://github.com/petroniusniger/ratatoskr/issues/7) accordingly.
- Added a draft formal test procedure (Github 
  [issue #14](https://github.com/petroniusniger/ratatoskr/issues/14)).
- Added confirmation pop-up to main 'ratatoskr' app to allow accepting or 
  rejecting incoming file transfer (Github 
  [issue #6](https://github.com/petroniusniger/ratatoskr/issues/6)).
  Pop-up dialog shows file name and size.
- Added `Changelog.md` (this file).
- Sanitize path for incoming files in main 'ratatoskr' app (Github 
  [issue #12](https://github.com/petroniusniger/ratatoskr/issues/12)).

## v0.1.0 (Feb. 17, 2026)

- First public release prior to code review by OpenStore team (Github only).
