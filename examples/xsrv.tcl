#! /bin/sh
# \
exec tclsh${TCL} "$0" "$@"

#
# Tulsa
#
# Exec'ing event-driven recho server.
#
#
# Stuart Cassoff
# Spring 2018
#

set path _Tulsa

package require tulsa
namespace import tulsa::tulsa

proc cfgcli {cli} {
	chan configure $cli -buffering line
	chan event $cli readable [list clihandler $cli]
}

proc clihandler {cli} {
	gets $cli r
	puts $cli [string reverse $r]
	if {$r ni {"" "x"}} { return }
	close $cli
	if {$r eq "x"} { set ::done 1 }
}

if {[llength $::argv] == 1} {
	cfgcli [tulsa tulsify [lindex $::argv 0]]
	# Since we're running in the bg and may not receive quit msg
	after 2000 [list set ::done 1]
	vwait done
	return
}

proc incoming {srv} {
	after cancel $::after_id
	set cli [tulsa accept $srv]
	chan configure $cli -closeonexec 0
	exec [auto_execok [info nameofexecutable]] [info script] [chan configure $cli -handle] &
	close $cli
	# Since we have no way of being told to quit
	set ::after_id [after 10000 [list set ::done 1]]
}

set srv [tulsa server $path]
chan event $srv readable [list incoming $srv]

# Since we have no way of being told to quit
set after_id [after 10000 [list set ::done 1]]

vwait done

close $srv

# EOF
