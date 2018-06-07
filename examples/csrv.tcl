#! /bin/sh
# \
exec tclsh${TCL} "$0" "$@"

#
# Tulsa
#
# Coroutine recho server.
#
#
# Stuart Cassoff
# Spring 2018
#

set path _Tulsa

package require tulsa
namespace import tulsa::tulsa

set srv [coroutine server apply {{path} {
	set ncli 0
	set srv [tulsa server $path]
	chan event $srv readable [info coroutine]
	yield $srv
	while 1 {
		coroutine cli[incr ncli] apply {{cli} {
			chan configure $cli -buffering line
			chan event $cli readable [info coroutine]
			yield
			while 1 {
				gets $cli r
				puts $cli [string reverse $r]
				if {$r in {"" "x"}} { break }
				yield
			}
			close $cli
			if {$r eq "x"} { set ::done 1 }
		}} [tulsa accept $srv]
		yield
	}
}} $path]

vwait done

close $srv

# EOF
