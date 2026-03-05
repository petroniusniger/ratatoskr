# Sprint #011: scope
(pan, v0, 27/02/2026)

Fix [issue #12](https://github.com/petroniusniger/ratatoskr/issues/12):
sanitize path for incoming file.

When the main 'ratatoskr' app processes an incoming file, its name (path) should
be checked for directory traversal attempts that would allow an attacker to try
and escape out of the cache (see `./ratatoskr/transfer.cpp` at l. 44).

The proposed approach is to remove any leading path element from the file
name (similar to what the `basename` command does when called with the file
name as only argument).

If a path element had to be removed (i.e. "original name" != "sanitized name"),
that event should be logged.
