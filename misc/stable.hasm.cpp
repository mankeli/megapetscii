#include <debugf.h>
#include <hashembler.h>

#include <math.h>
#include "../lib.h"


using namespace hashembler;

segment_basic_c basicstub;
segment_asm_c initprg;

void stub()
{
	basicstub.begin(0x801);
	basicstub.add_sys(666, SEGLABEL(initprg, "begin"));
	basicstub.add_end();

	initprg.begin(SEGPC(basicstub));
	set_segment(&initprg);
	LPC("begin");

	SEIa();
	LDAi(0x7f);
	STA(0xdc0d);
	STA(0xdd0d);
	LDA(0xdc0d);
	LDA(0xdd0d);




	INC(0xd020);
	MOV8i(4, 0);


	// cia interval
	LDAi(62); STA(0xdd04); LDAi(00); STA(0xdd05);

	// wait for upper part of screen
	LDA(0xD011); ANDi(0x7F); LPC("wait0") CMP(0xd011); BNE(L("wait0"))

	// wait for lower part of screen
	LDA(0xD011); ORAi(0x80); LPC("wait1") CMP(0xd011); BNE(L("wait1"))

	// wait for line
	LDAi(0x10); LPC("wait2"); CMP(0xd012); BNE(L("wait2"));

	//INC(0xd020);

	LDYi(0x91);
	NOPa();
	NOPa();
	NOPa();
	NOPa();
	NOPa();
	NOPa();

	value_t var = 0x10;
	LDAi(0);
	STAz(var);
	LPC("syncloop")
	LDA(0xd012) // read at cycle 4
	CMPz(var)   // 3
	BEQ(L("now_out")) // 2
	// 62-(4+3+2)-(4+2+9*(2+3)-1+3)
	STA(var); // 4
	LDXi(9); // 2
	LPC("cyclewaster")
		DEXa();  // 2
		BNE(L("cyclewaster")); // 3 (-1 on last)
	JMP(L("syncloop")) // 3

	LPC("now_out")
	// here we are. scanline start + 3 + 3 cycles.

	STY(0xdd0e)

	PCNOW("cia started")


	INC(0xd020);


	LDXi(0);
	LPC("memfill_loop")
	LDAi(1);
	STAx(0xD800)
	TXAa();
	STAx(0xD900)
	STAx(0xDA00)
	STAx(0xDB00)
	EORi(0xAA);
	STAx(0x400)
	STAx(0x500)
	STAx(0x600)
	STAx(0x700)
	INXa();
	BNE(L("memfill_loop"))


	MOV8i(0xd011, 0x1B);


	MOV8i(0xD01A, 0x01);
	MOV8i(0xD012, 0x31);
	MOV16i(0x0314, L("irqrut_kernal"));
	CLIa();

	value_t screenpos = 0x10;
	MOV8i(screenpos, 0x15);

	value_t scrollpos = 0x11;
	MOV8i(scrollpos, 0x07);

	value_t lastpressed = 0x00;


	LPC("loop")

	INC(2)
	LDA(2)
	STA(3)
	LPC("loop2")
	INC(3)
	BEQ(L("loop2"))

#if 0
	LDA(0xDC00)
	ANDi(0x1F)
	PHAa();
	EORi(0x1F)
	ANDz(lastpressed);

	CMPi(0x10);
	BNE(L("nosongchange"))
	PLAa();
	JMP(L("begin"))
	LPC("nosongchange");

	CMPi(0x08);
	BNE(L("noleft"))
	INC(screenpos)
	LPC("noleft");

	CMPi(0x04);
	BNE(L("noright"))
	DEC(screenpos)
	LPC("noright");

	PLAa();
	STAz(lastpressed);

#endif

	LDAi(0x70);
	CMP(0xd012);

	JMP(L("loop"))




/*	INC(2)
	LDA(2)
	STA(3)
	LPC("loop2")
	INC(3)
	BEQ(L("loop2"))
*/
	LDAi(0x10);
	CMP(0xd012);
	BNE(L("loop"))
	DEC(0xd020);
	JMP(L("loop"))


	PAGE;
	LPC("irqrut_kernal");
	LSR(0xd019);

	LDAi(0x7F)
	//LDAi(0x10)
	STA(0xd011)
	STA(0xd020)

#if 0
	LDAz(scrollpos)
	ANDi(0x07);
	STA(0xd016);
	LDAz(scrollpos)
	LSRa();
	LSRa();
	LSRa();
	CLCa();
	ADCi(0x15);
	STAz(screenpos)
#endif

	int i;

	LDAi(0x32);
	LPC("wait_for_screen_begin")
	CMP(0xd012);
	BNE(L("wait_for_screen_begin"))
	INC(0xd020);

#if 1
	LDA(0xdd04)				// kestää 4 sykliä
	CLCa();
	ADCz(screenpos);
	LSRa();					// jako / 2:lla, eka bitti = C
	BCS(L("onecycle")) 		// 2 sykliä normisti, jos C=1 nii 3
	LPC("onecycle");

	EORi(0x3f);
	STA(L("bccmod") + 1) 	// 4 sykliä
	//CLCa();					// 2 sykliä
	LPC("bccmod")
	BPL(L("bccend"));		// hyppää eteenpäin cia arvo / 2 byteä, eli int(cia/2)*2 sykliä
	LPC("bccend")

	PCNOW("nopshit")
	//for (i = 0; i < 0x93; i++)
	//for (i = 0; i < 0x54; i++)
	for (i = 0; i < 0x28; i++)
		B(0xEA);
	PCNOW("nopshit_end")
	LDAz(0)

	LDAi(0x1B);
//	LDAi(0x17);
	STA(0xd011)
//	STA(0xd020)

//	for (i = 0; i < 400; i++)
//		B(0xEA);

#endif


	LDAz(screenpos)
	LDYi(0)
	JSR(L("drawhex"))

	LDA(L("bccmod") + 1)
	LDYi(5)
	JSR(L("drawhex"))

//	MOV8i(0xd011, 0x1F);



	LDA(0xDC00)
	ANDi(0x1F)
	PHAa();
	EORi(0x1F)
	ANDz(lastpressed);

	CMPi(0x10);
	BNE(L("nosongchange"))
	PLAa();
	JMP(L("begin"))
	LPC("nosongchange");

	CMPi(0x08);
	BNE(L("noleft"))
	INC(screenpos)
	LPC("noleft");

	CMPi(0x04);
	BNE(L("noright"))
	DEC(screenpos)
	LPC("noright");

	PLAa();
	STAz(lastpressed);



	LDAi(0)
	STA(0xd020);

	JMP(0xEA81);


	LPC("drawhex")
	PHAa();
	LSRa();
	LSRa();
	LSRa();
	LSRa();
	ANDi(0x0F);
	TAXa();
	LDAx(L("hextab"));
	STAy(0x400 + 40 * 2);

	PLAa();
	ANDi(0x0F);
	TAXa();
	LDAx(L("hextab"));
	STAy(0x400 + 40 * 2+1);
	RTSa();


	// data for hex conversion & loading
	LPC("hextab");
	for (i = 0; i < 16; i++)
	{
		int as;
		if (i <= 9)
			as = 0x30+i;
		else
			as = i - 10 + 1;
		B(as);
	}


	LPC("sinus")
	for (i = 0; i < 256; i++)
	{
		int v = 0x80 + sin(i * (M_PI / 128.f)) * 0x70;
		B(v);
	}


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
	make_prg("stable.prg", 0x0801, segs);
}
