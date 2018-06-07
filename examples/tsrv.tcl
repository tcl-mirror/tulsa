#! /bin/sh
# \
exec tclsh${TCL} "$0" "$@"

#
# Tulsa
#
# Threaded recho server.
#
#
# Stuart Cassoff
# Spring 2018
#

set path _Tulsa
set nthreads 0

package require tulsa
namespace import tulsa::tulsa
package require Thread

proc incoming {srv} {
	set cli [tulsa accept $srv]
	chan configure $cli -buffering line
	after idle [list gogo $cli]
}

proc gogo {cli} {
	lassign [tulsa pair] hand finger
	chan configure $hand -buffering line
	chan configure $finger -buffering line
	set tid [thread::create -joinable]
	thread::send $tid { package require tulsa }
	thread::transfer $tid $hand
	thread::send $tid "set hand $hand"
	thread::transfer $tid $cli
	thread::send $tid "set cli $cli"
	chan event $finger readable [list legerdemain $finger $tid]
	thread::send $tid {
		chan event $cli readable [list apply {{cli hand} {
			gets $cli r
			puts $cli [string reverse $r]
			if {$r ni {"" "x"}} { return }
			close $cli
			puts $hand $r
			gets $hand cmd
			close $hand
			eval $cmd
		}} $cli $hand]
	}
	incr ::nthreads
}

proc legerdemain {finger tid} {
	gets $finger r
	puts $finger thread::release
	close $finger
	thread::join $tid
	incr ::nthreads -1
	if {$r eq "x"} { set ::done 1 }
}

set srv [tulsa server $path]
chan event $srv readable [list incoming $srv]

vwait done

close $srv

#puts nthreads:\ $nthreads\n\[thread::names\]:\ [thread::names]

# EOF
