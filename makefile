HASM=~/src/hashembler/hasm.sh
LZ=~/src/doynamite1.1/lz

DISK1=HACKER~1.D64
DISK2=HACKER~2.D64

all: $(DISK1)

$(DISK1): stub.prg actual.prg.lz msdos.prg.lz makspein.prg.lz cumvm.prg.lz mobs.prg.lz maked64.sh megapetscii/megap.tar
	./maked64.sh

stub.prg.lz: stub.prg
	$(LZ) --sfx 0x80e -o $@ $^

makspein.prg.lz: bin/makspein28.prg
	$(LZ) -o $@ $^

msdos.prg.lz: msdos.prg
	$(LZ) -o $@ $^

actual.prg.lz: actual.prg
	$(LZ) -o tmp.lz $^
	megapetscii/fix_loadaddr tmp.lz $@

cumvm.prg.lz: cumvm.prg
	$(LZ) -o $@ $^

mobs.prg.lz: bin/mobs.prg
	$(LZ) -o tmp.lz $^
	megapetscii/fix_loadaddr tmp.lz $@

stub.prg: stub.hasm.cpp krill/install-c64.prg krill/loader-c64.prg krill.h
	$(HASM) $<

msdos.prg: msdos.hasm.cpp spiralclear.cpp krill.h
	$(HASM) $< $(word 2,$^)

actual.prg: actual.hasm.cpp krill.h
	$(HASM) $<

cumvm.prg: cumvm.hasm.cpp script.cum krill.h
	$(HASM) $<

clean:
	rm *.prg.lz *.prg $(DISK1) $(DISK2)
