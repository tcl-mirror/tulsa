#! /bin/sh
# \
exec tclsh${TCL} "$0" "$@"

#
# Tulsa
#
# Event-driven pair channels one-upping each other.
#
#
# Stuart Cassoff
# Spring 2018
#

set path _Tulsa
set max 1000

package require tulsa
namespace import tulsa::tulsa

proc count {chan} {
	gets $chan count
	if {$count < $::max} {
		puts $chan [incr count]
	} else {
		set ::result $count
	}
}

set clis [tulsa pair]

foreach cli $clis {
	chan configure $cli -buffering line
	chan event $cli readable [list count $cli]
}

# Go
puts [lindex $clis 0] 0

vwait result

foreach cli $clis {
	close $cli
}

puts $result

# EOF
