#!/bin/bash

#DFLAGS=--best-offset-tables
#DFLAGS="--window 0x200"

rm prepare_screens
clang++ prepare_screens.cpp -o prepare_screens
clang fix_loadaddr.c -o fix_loadaddr

rm -rf out/* packed/* megap.tar
mkdir -p out packed
./prepare_screens megapetscii.c $1
for i in `ls out`; do
	echo "crunching $i"
	DFLAGS_L=$DFLAGS
	#if [ "$i" = "24.pet" ]; then DFLAGS_L="$DFLAGS_L --best-offset-tables"; fi
	#if [ "$i" = "2b.pet" ]; then DFLAGS_L="$DFLAGS_L --best-offset-tables"; fi
	echo ~/src/doynamite1.1/lz $DFLAGS_L -o packed/$i out/$i
	~/src/doynamite1.1/lz $DFLAGS_L -o packed/${i} out/$i
	./fix_loadaddr packed/${i} packed/${i}.lz
	rm packed/${i}

	#~/src/bb/bb out/$i
	#mv out/${i}.bb packed/
done
tar -cvf megap.tar packed/
ls -l megap.tar
