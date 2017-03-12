# MegaPETSCII
#### A C64 demo from HACKERS

Ok this is the full source for MegaPETSCII c64 demo by HACKERS. This source includes the MegaPETSCII preprocessor software and the actual runtime client for C64. The iPad client apparently still has some business relevance so it's not included.

## Build requirements:
* Hashembler (https://github.com/mankeli/hashembler/)
  * Oct 27, 2016, commit 40a47d1d is confirmed to work
* Doynamite cruncher v1.1 (http://csdb.dk/release/?id=129574)
* c1541 (http://vice-emu.sourceforge.net/)
* Linux/OSX with clang++

## Build instructions:
* Get all the prerequisite programs
* Have c1541 binary on $PATH
* Fix paths to utilities in main Makefile and megapetscii/doit.sh
* Go to megapetscii/ and run ./really_doit.sh – this constructs the MegaPETSCII data bundles
* say "make" in main directory
* If everything went better than expected, you should have HACKER~1.D64 in the directory
* say "x64 +truedrive HACKER~1.D64" and enjoy!

## Credits:

* Main Program by Ash Checksum/HACKERS
* Direction, 99.576% of graphics and screenplay and idea by Hi-Stack/HACKERS
* Sound Engineering by Van Damme/HACKERS
* Additional Design by Zero Cool/HACKERS


* Loader by Krill/Plush
* Cruncher by Doynax and Bitbreaker/Oxyron
* Music routine by Cadaver/Covert Bitops
* Petscii editor by Marq/FIT

