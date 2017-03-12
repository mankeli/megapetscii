#!/bin/bash

function maked64 {
	CMD=$1
	BUNDLES=$2
	for i in $BUNDLES
	do
		#d_i="$(echo $i | rev | cut -d / -f 1 | rev)z"
		CMD="$CMD
		write $i"
	done
	#CMD="$CMD
	#write 21.pet.lz"

	echo "$CMD" | c1541 > /dev/null
}

bundles=$(find megapetscii/packed -type f | sort)

maked64 "
format hackers,69 d64 HACKER~1.d64
write stub.prg
write cumvm.prg.lz
write actual.prg.lz
write msdos.prg.lz
write makspein.prg.lz
write bin/mobs.prg
" "$bundles"
