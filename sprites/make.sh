#!/bin/bash
echo -en "\000" > mobs.prg
echo -en "\060" >> mobs.prg
cat car >> mobs.prg
mv mobs.prg ../bin/