#include <debugf.h>
#include <hashembler.h>

#include <math.h>
#include "lib.h"


using namespace hashembler;

segment_basic_c basicstub;
segment_asm_c initprg;

segment_c krill_install;
segment_c krill_resident;

#include "krill.h"

void stub()
{
	CTX("stub")
	krill_install.load_prg("krill/install-c64.prg", true);
	krill_resident.load_prg("krill/loader-c64.prg", true);

	printf("install %04X-%04X load %04X-%04X\n",
		SEGLABEL(krill_install, "start"), krill_install.m_pc,  SEGLABEL(krill_resident, "start"), krill_resident.m_pc);

	basicstub.begin(0x801);
	basicstub.add_sys(666, SEGLABEL(initprg, "begin"));
	basicstub.add_end();

	initprg.begin(SEGPC(basicstub));
	set_segment(&initprg);
	LPC("begin");
	SEGLABEL(initprg, "begin") = L("begin");
	JMP(L("begin2"))

	LPC("loaderror");
	STX(0xd020);
	STA(0xd020);
	LDXi(0x02);
	JMP(L("loaderror"));

	LPC("begin2");

	SEI();
	LDAi(0x7f);
	STA(0xdc0d);
	STA(0xdd0d);
	LDA(0xdc0d);
	LDA(0xdd0d);

	JSR(SEGLABEL(krill_install, "start"))
	BCS(L("loaderror"))

	// krill seems to enable kernal anyway when it needs it,
	// so this enables compatibility to load under BASIC/KERNAL with 1541
	LDAi(0x35);
	STAz(0x01);

/*
	LDXi(LB(L("somefn")));
	LDYi(HB(L("somefn")));
//	JSR(krill_load)
	JSR(krill_load_packed)
	BCS(L("loaderror"))	

	B(2);

	LDAi(0x35);
	STAz(0x01);
*/

	LDXi(LB(L("cumvmfn")));
	LDYi(HB(L("cumvmfn")));
//	JSR(krill_load)
	JSR(krill_load_packed)
	BCS(L("loaderror"))

	LDAi(0x35);
	STAz(0x01);

//	INC(0xd021);
	LDXi(LB(L("actualprgfn")));
	LDYi(HB(L("actualprgfn")));
	JSR(krill_load_packed)
	BCS(L("loaderror"))

	LDAi(0x35);
	STAz(0x01);

#if 0
	INC(0xd021);
	LDXi(LB(L("fn3")));
	LDYi(HB(L("fn3")));
	JSR(SEGLABEL(krill_resident, "start") + 3)
	BCS(L("loaderror"))

	LDAi(0x35);
	STAz(0x01);
#endif

//	INC(0xd021);
	LDXi(LB(L("songfn")));
	LDYi(HB(L("songfn")));
	JSR(krill_load_packed)
	BCS(L("loaderror"))

	LDAi(0x35);
	STAz(0x01);

	PRINTPOS("mobs load")

	LDXi(LB(L("mobsfn")));
	LDYi(HB(L("mobsfn")));
	JSR(krill_load)
	BCS(L("loaderror"))

	LDAi(0x35);
	STAz(0x01);

//	DEC(0xd021);
//	DEC(0xd021);

	//B(2);

#ifdef SKIP_INTRO
	MOV16i(framecounter, 0x0E00);
	//MOV16i(framecounter, 0x3000);
	JMP(actual_start);
	LPC("actualprgfn")
	SEG->add_string("ACTUAL.PRG.LZ");
#else
	MOV16i(framecounter, 0x0000);
	JMP(msdos_start);
	LPC("actualprgfn")
	SEG->add_string("MSDOS.PRG.LZ");
#endif

	LPC("songfn")
	SEG->add_string("MAKSPEIN.PRG.LZ");

	LPC("cumvmfn")
	SEG->add_string("CUMVM.PRG.LZ");

	LPC("somefn")
	SEG->add_string("21.PET.LZ");

	LPC("mobsfn")
	SEG->add_string("MOBS.PRG");

	PRINTPOS("end of dada?")
}

void genis()
{
	stub();
}


int main()
{
	assemble(genis);

	list<segment_c *> segs;
	segs.push_back(&basicstub);
	segs.push_back(&initprg);
	segs.push_back(&krill_install);
	segs.push_back(&krill_resident);
	make_prg("stub.prg", 0x0801, segs);
}
