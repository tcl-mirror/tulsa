#! /bin/sh
# \
exec tclsh${TCL} "$0" "$@"

#
# Tulsa
#
# Event-driven recho server.
#
#
# Stuart Cassoff
# Spring 2018
#

set path _Tulsa

package require tulsa
namespace import tulsa::tulsa

proc incoming {srv} {
	set cli [tulsa accept $srv]
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

set srv [tulsa server $path]
chan event $srv readable [list incoming $srv]

vwait done

close $srv

# EOF
