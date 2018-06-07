#! /bin/sh
# \
exec tclsh${TCL} "$0" "$@"

#
# Tulsa
#
# Recho client.
#
# Call with more than one arg to tell the server to exit.
#
#
# Stuart Cassoff
# Spring 2018
#

set path _Tulsa

set l [list Hello Goodbye]
if {$argc > 0} {
	lappend l x	;# Tell server to exit
} else {
	lappend l ""	;# Tell server to close
}

package require tulsa
namespace import tulsa::tulsa

set cli [tulsa client $path]
chan configure $cli -buffering line

set rl [list]
foreach s $l {
	puts $cli $s
	lappend rl [gets $cli]
}

close $cli

puts $l
puts $rl

# EOF
