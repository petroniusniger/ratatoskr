# Sprint #012: scope
(pan, v0, 05/03/2026)


Fix [issue #6](https://github.com/petroniusniger/ratatoskr/issues/6):
Add confirmation pop-up before accepting incoming file over BT.

In the 'ratatoskr' main app, when an incoming transfer is detected, and prior
to saving the incoming file in the cache, a pop-up dialog should be presented
to the user. The dialog should contain the following:

- a sentence stating that an incoming BT transfer has been detected
- the name of the incoming file
- (if available) its size
- (if available) the name of the sending device
- an "Accept" button
- a "Reject" button

If the "Accept" button is pressed, the incoming file should be saved to the 
cache (following the existing workflow).

If the "Reject" button is pressed, the incoming transfer should be aborted 
gracefully (i.e. in a way that would allow the originating device to send 
further files to the phone without the need for any process to be restarted).
