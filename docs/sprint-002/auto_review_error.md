# Error issued by the automated review process
(pan, 18/12/2025)

Issuing the `clickable build` command successfully builds the software in its
current state, but the automated review process at the end produces an error 
for each executable:

```
Running review on /home/philipa/copilot/ratatoskr/ratatoskr/build/x86_64-linux-gnu/app/ratatoskr.philipa_251218160426_amd64.click
Errors
------
 - security:template_valid:ratatoskr.apparmor
	(NEEDS REVIEW) 'unconfined' not allowed
	http://askubuntu.com/q/671403
 - security:template_valid:shareplugin.apparmor
	(NEEDS REVIEW) 'unconfined' not allowed
	http://askubuntu.com/q/671403
/home/philipa/copilot/ratatoskr/ratatoskr/build/x86_64-linux-gnu/app/ratatoskr.philipa_251218160426_amd64.click: FAIL
Command exited with non-zero exit status 2, see above for details. This is most likely not a problem with Clickable.
```
