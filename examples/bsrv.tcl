#! /bin/sh
# \
exec tclsh${TCL} "$0" "$@"

#
# Tulsa
#
# Blocking recho server.
#
#
# Stuart Cassoff
# Spring 2018
#

set path _Tulsa

package require tulsa
namespace import tulsa::tulsa

set srv [tulsa server $path]

while 1 {
	set cli [tulsa accept $srv]
	chan configure $cli -buffering line
	while 1 {
		gets $cli r
		puts $cli [string reverse $r]
		if {$r in {"" "x"}} { break }
	}
	close $cli
	if {$r eq "x"} { break }
}

close $srv

# EOF
