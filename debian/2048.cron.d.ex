#
# Regular cron jobs for the 2048 package
#
0 4	* * *	root	[ -x /usr/bin/2048_maintenance ] && /usr/bin/2048_maintenance
