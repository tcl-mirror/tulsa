#! /bin/sh
# \
exec tclsh${TCL} "$0" "$@"

#
# Tulsa
#
# Your system's timeout granularity.
#
#
# Stuart Cassoff
# Spring 2018
#

package require tulsa

set which -sendtimeout
#set which -receivetimeout

lassign [tulsa::tulsa pair] s1 s2

puts $which
puts default:[chan configure $s1 $which]

foreach t [list 0 1 3.51230000 555.911 1.5 120 100.1234] {
	chan configure $s1 $which $t
	puts [string cat "set: " $t "  got: " [chan configure $s1 $which]]
}

# EOF
