#include <debugf.h>
#include <hashembler.h>

#include <math.h>
#include "lib.h"

#include "krill.h"

namespace spiral
{
	extern char tab[40*25];
	extern uint16_t startpoints[1000];
	extern int startpointcount;
	int make(void);
};

using namespace hashembler;

static float fracf(float val)
{
	return val - floor(val);
}

void write_colortab(int *t1, int numcol, int mode)
{
	int i;
	for (i = 0; i < 256; i++)
	{
		float intensity = (float)i / 255.f;
		if (mode == 1)
			intensity = 0.5f + 0.5f * cos(M_PI + intensity * 5.f * M_PI);
		intensity += (((float)(rand() & 0xFFFFFF) / (float)(0xFFFFFF)) - 0.5f) * 0.4f;
		float fnum = intensity * (numcol - 1.01f);

		int num = round(fnum);
		if (num < 0)
			num = 0;
		else if (num > (numcol - 1))
			num = numcol - 1;

		B(t1[num])
	}
}


segment_basic_c basicstub;
segment_asm_c initprg;
segment_asm_c actualprg;

value_t chg = 2;

value_t coloffs = 0x4; // siz 2
value_t coloffs2 = 0x6; // siz 2
value_t tmp1 = 0x8;
value_t tmp2 = 0x9;
value_t dronescreen = 0xA; // siz 2
value_t nextdrone = 0xC;
value_t d1 = 0xD; // siz 2
value_t nextdroneframe = 0x0F;
value_t d2_h = 0x10;
value_t d2_l = 0x18;
value_t nextdroneframe_v = 0x20;

int dronecount = 8;

value_t nextwriterframe = 0x23;
value_t writerptr = 0x24; // siz 2
value_t cursor_x = 0x26;
value_t writerspeed = 0x27;
value_t writer_run = 0x28;

void msdoseffu()
{
	CTX("msdos")
	actualprg.begin(msdos_start);
	set_segment(&actualprg);

	LPC("begin");
	PRINTPOS("begin");

//	LDAi(0);
//	JSR(0xE000);

	LDAz(0x01);
	PHA();
	LDAi(0x35);
	STAz(0x01);
	LDAi(0x00);
	JSR(music_init);
	PLA();
	STAz(0x01);

	LDAi(0x37);
	STAz(0x01);

	SEI();
	LDAi(0x7f);
	STA(0xdc0d);
	STA(0xdd0d);
	LDA(0xdc0d);
	LDA(0xdd0d);



	LDXi(0);
	LPC("normalizeloop")
	int i;
	for (i = 0; i < 4; i++)
	{
		LDAx(0x400 + i * 0x100)
		if (i >= 2)
		{
//			TXA();
		}
		ANDi(0x3F)
//		LDAi(0x20);
		STAx(0x400 + i * 0x100)
	}
	INX();
	BNE(L("normalizeloop"))

	LDA(0xd011)
	ORAi(0xC0)
	STA(0xd011)


	LDAi(0x6)
	STA(0xd021);
	LDAi(0x4)
	STA(0xd022);
	LDAi(0xe)
	STA(0xd023);
	LDAi(0xe)
	STA(0xd024);


	LDAi(0)
	for (i = 0; i < dronecount; i++)
	{
		STAz(d2_h + i);
	}


	LDAi(0xff)
	STAz(nextdrone)
	LDAi(30)
	STAz(nextdroneframe)
	LDAi(0x20)
	STAz(nextdroneframe_v)

	LDAi(220)
	STAz(nextwriterframe)

	LDAi(LB(L("text")))
	STAz(writerptr + 0)
	LDAi(HB(L("text")))
	STAz(writerptr + 1)



	MOV8i(0xD01A, 0x01);
	MOV8i(0xD012, 0x30);
	MOV16i(0x0314, L("irqrut_kernal"));
	MOV16i(0xFFFE, L("irqrut_direct"));
	CLI();


	LDAi(0)
	STAz(writer_run)


	LPC("jumi")

	LDAz(writer_run)
	BEQ(L("jumi"))
	LDAi(0)
	STAz(writer_run)
#if 1
	{


		LDYi(0);
		LDAizy(writerptr)
		CMPi(0x70)
		BNE(L("wr_1"))

		JSR(L("carriage"))
		LDAi(0)
		STAz(cursor_x)

		JMP(L("writer_inc_ptr"))

		LPC("wr_1")
		CMPi(0x71)
		BNE(L("wr_2"))
		LDAi(6);
		STAz(writerspeed)
		JMP(L("writer_inc_ptr"))

		LPC("wr_2")
		CMPi(0x72)
		BNE(L("wr_3"))


		LDAi(1);
		STAz(writerspeed)
		JMP(L("writer_inc_ptr"))

		LPC("wr_3")
		CMPi(0x73)
		BNE(L("wr_4"))
		LDAi(40);
		STAz(writerspeed)
		JMP(L("writer_inc_ptr"))

		LPC("wr_4")
		CMPi(0x74)
		BNE(L("wr_5"))
		JMP(L("writer_inc_ptr"))

		LPC("wr_5")
		CMPi(0x75)
		BNE(L("wr_6"))
		JSR(L("load_nextpart"))
		JMP(L("writer_inc_ptr"))

		LPC("wr_6")
		CMPi(0x76)
		BNE(L("wr_7"))
		JSR(L("nextpart"))
		JMP(L("writer_inc_ptr"))

		LPC("wr_7")

		LDXz(cursor_x)
		LDAx(0x400 + 24 * 40)
		ANDi(0xC0);
		ORAizy(writerptr)
		STAx(0x400 + 24 * 40)
		LDAi(1)
		STAx(0xd800 + 24 * 40)

		INX();
		STXz(cursor_x)


		LPC("writer_inc_ptr")

		ADD16z_i(writerptr, 1);
		LDAz(framecounter + 0)
		ADCz(writerspeed);
		STAz(nextwriterframe)
	}
#endif
	JMP(L("jumi"))

	LPC("irqrut_kernal");
	JSR(L("irqrut"))
	JMP(0xEA81);

	LPC("irqrut_direct");
	PHA();
	TXA();
	PHA();
	TYA();
	PHA();
	JSR(L("irqrut"))
	PLA();
	TAY();
	PLA();
	TAX();
	PLA();
	RTI();

	LPC("irqrut");
	LSR(0xd019);

//	;

	ADD16z_i(framecounter, 1);

#if 1
	LDAz(framecounter + 1)
	CMPi(0x02);
	BCC(L("nobordercolupdate"))
	CMPi(0x03);
	BCS(L("nobordercolupdate"))
	LDXz(framecounter + 0)
	LDAx(L("cta1"))
	STA(0xd020)
	STA(0xd023)
	LDAx(L("cta2"))
	STA(0xd022)
	LPC("nobordercolupdate")

	LDAz(framecounter + 1)
	CMPi(0x00);
	BNE(L("nofirstcolupdate"))
	LDXz(framecounter + 0)
	LDAx(L("cta3"))
	STA(0xd022)
	LPC("nofirstcolupdate")
#endif


	LDAi(0x35);
	STAz(0x01);
	JSR(music_play);
	LDAi(0x37);
	STAz(0x01);


	LDAz(framecounter + 0)
	CMPz(nextdroneframe)
	BEQ(L("part_drones"))
	JMP(L("no_part_drones"))
	LPC("part_drones");

	DECz(nextdroneframe_v)
	BNE(L("v_is_fine"))
	INCz(nextdroneframe_v)
	LPC("v_is_fine")
	LDXz(nextdroneframe_v)
	CLC();
	ADCx(L("dronespeedtab"));
	STAz(nextdroneframe)

	{
#if 1
	for (i = 0; i < dronecount; i++)
	{
		LDAz(d2_l + i)
		STAz(d1 + 0)
		LDAz(d2_h + i)
		STAz(d1 + 1)
		JSR(L("droneone"))
		LDAz(d1 + 0)
		STAz(d2_l + i)
		LDAz(d1 + 1)
		STAz(d2_h + i)
	}
#endif
	}
	LPC("no_part_drones");

	;
#if 1
	LDAz(framecounter + 0)
	CMPz(nextwriterframe)
	BEQ(L("part_writer"))
	JMP(L("no_part_writer"))
	LPC("part_writer");
	INCz(writer_run)

#endif

	LPC("no_part_writer")

//	DEC(0xd020);

	RTS();

	LPC("load_nextpart")
/*
		SEI();
		MOV8i(0xD418, 0x00);
		MOV8i(0xD01A, 0x00);
		LSR(0xd019);
		MOV16i(0x0314, 0xEA81);
*/


		LDXi(LB(L("actualprgfn")));
		LDYi(HB(L("actualprgfn")));
		JSR(krill_load_packed)
		RTS();

//		LDAi(0x35);
//		STAz(0x01);
		LPC("actualprgfn")
		SEG->add_string("ACTUAL.PRG.LZ");

	LPC("nextpart")
		LDXi(0);
		TXA();
		LPC("clearcolmemloop")
		STAx(0xd800);
		STAx(0xd900);
		STAx(0xdA00);
		STAx(0xdB00);
		INX();
		BNE(L("clearcolmemloop"))

		JMP(actual_start)
		RTS();


	LPC("carriage")
	{
		int i;
		for (i = 1; i < 25; i++)
		{
			value_t c1 = 0x400+(i-1)*40;
			value_t c2 = 0x400+i*40;
			LDAi(LB(c2))
			STAz(coloffs)
			LDAi(HB(c2))
			STAz(coloffs + 1)
			LDAi(LB(c1))
			STAz(coloffs2)
			LDAi(HB(c1))
			STAz(coloffs2 + 1)
			JSR(L("movecol"))

			c1 = 0xD800+(i-1)*40;
			c2 = 0xd800+i*40;
			LDAi(LB(c2))
			STAz(coloffs)
			LDAi(HB(c2))
			STAz(coloffs + 1)
			LDAi(LB(c1))
			STAz(coloffs2)
			LDAi(HB(c1))
			STAz(coloffs2 + 1)
			JSR(L("movecol"))

		}
		for (i = 0; i < 40; i++)
		{
			LDA(0x400 + 24 * 40 + i)
			ANDi(0xc0);
			ORAi(0x20);
			STA(0x400 + 24 * 40 + i)
		}
		RTS();
	}

	LPC("movecol")
//	;
	LDYi(0);
	for (i = 0; i < 40; i++)
	{
		LDAizy(coloffs);
		ANDi(0x3f);
		STAz(tmp1)

		LDAizy(coloffs2);
		ANDi(0xC0);
		ORAz(tmp1);
		STAizy(coloffs2);

		INY();
	}
	RTS();





	LPC("text")
	const char *text = "\b\nSHH....\f\t\t\t\n"
	"\bNO TEARS\f\t\t\t\b\n\n"
	"\bONLY DREAMS NOW....\f\t"
	"\n\n\b\n\n\n"
	"\bSTARTING MS-DOS...\n\f\t\t\t\b\n\nHIMEM IS TESTING EXTENDED MEMORY...\f\t\bNONE\n\f\t\b\n\n"
	"MICROSOFT(R) MS-DOS(R) VERSION 6.21\n"
	"(C)COPYRIGHT MICROSOFT CORP 1981-1993\n"
	"C64 PORT BY ASH CHECKSUM OF HACKERS\n"
	"\n"
//	"0123456789012345678901234567890123456789"
	"\t\t\t\t\t\n"
	"A:\\>\f\t\t\t\aDIR\b\n"
	"\f\t\t\t\t\b"
	" VOLUME IN DRIVE A IS CLOUD-MEGAPETSCII\n"
	" VOLUME SERIAL NUMBER IS 1337-9876\n"
	" DIRECTORY OF A:\n"
	"\n"
	"\n"
	".             <DIR>     13-07-1985 23:29\n"
	"..            <DIR>     13-07-1985 23:29\n"
	"DEMO3    EXE        666 12-10-2014 16:20\n"
	"README   TXT        100 12-10-2014 16:20\n"
	"ZOO15DEM EXE        666 26-09-2015 16:20\n"
	"\n"
	"        2 FILE(S)        191 BLOCKS FREE\n"
	"\n\t\t\t\t\t\t\t\t\t"
	"A:\\>\a\t\t\t\t\t\aTYPE README.TXT\b\n"
	"\f\t\t\b\n"
	"MEGAPETSCII BY HACKERS\n\n"
	"A SPARSE VIRTUAL TEXTURE FOR THE C64\n"
	"HOME COMPUTER\n\n"
	"MAIN PROGRAM     ASH CHECKSUM OF HACKERS\n"
	"GRAPHIC DIRECTOR     HI-STACK OF HACKERS\n"
	"SOUND ENGINEER      VAN DAMME OF HACKERS\n"
	"ADD. DESIGN         ZERO COOL OF HACKERS\n"
	"\n"
	"LOAD PROGRAM              KRILL OF PLUSH\n"
	"CRUNCH PROGRAM     DOYNAX AND BITBREAKER\n"
	"SOUND PROGRAM   CADAVER OF COVERT BITOPS\n"
	"PETSCII EDITOR               MARQ OF FIT\n"
	"\n\t\t\t\t\t\t\t\t\t\n"
	"A:\\>\a\t\t\t\t\t\aDEMO\b\n"
	"\f\t\t\t\t\t\b\n"
	"BAD COMMAND OR FILE NAME\n"
	"\n\f\t\t\b\n"
	"A:\\>\a\t\t\t\t\t\aDEMO3\b\n"
	"\r"

	"\n\n\n\n    \bENTERING CYBER WORLD....\a\t\t\n\n\n\n\n\n\b\n\n\n\n\n\n\n\t\t\n\n\n\n\n\n\n\n\n\n\n\n\n"

	"\v";

	for (i = 0; i < strlen(text); i++)
	{
		if (text[i] == '\n')
			B(0x70)
		else if (text[i] == '\a')
			B(0x71)
		else if (text[i] == '\b')
			B(0x72)
		else if (text[i] == '\f')
			B(0x73)
		else if (text[i] == '\t')
			B(0x74)
		else if (text[i] == '\r')
			B(0x75)
		else if (text[i] == '\v')
			B(0x76)
		else if (text[i] == '\\')
			B('/')
		else
			B((text[i] & 0x3F))
	}



	LPC("droneone")

		LDA(d1 + 1)
		BNE(L("nodronereload"))

		LDXz(nextdrone);
		INX();
		CPXi(spiral::startpointcount)
		BNE(L("dodrones"))

		JMP(L("skipdrones"));

		LDXi(0x00)

		LPC("dodrones");

		STXz(nextdrone);
		LDAx(L("dronestarts_h"))
		STAz(d1 + 1)

		LDAx(L("dronestarts_l"))
		STAz(d1 + 0)


		LPC("nodronereload");

		// set up drone screenspace address
		value_t maptoscreen = (0x10000 + 0x400 - L("dronemap")) & 0xFFFF;
		printf("map to screen %04X\n", maptoscreen);
		LDA(d1 + 0);
		CLC();
		ADCi(LB(maptoscreen));
		STA(dronescreen + 0)

		LDA(d1 + 1);
		ADCi(HB(maptoscreen));
		STA(dronescreen + 1)

		LDYi(0);

		// alter screen
		LDAizy(dronescreen);
		CLC();
		ADCi(0x40);
		STAizy(dronescreen);

		LDAizy(d1);

		LPC("dir1")
		CMPi('^');
		BNE(L("dir2"))
		LDAz(d1 + 0);
		SEC();
		SBCi(40);
		STAz(d1 + 0);
		LDAz(d1 + 1);
		SBCi(0);
		STAz(d1 + 1);
		JMP(L("nodir"))

		LPC("dir2")
		CMPi('>');
		BNE(L("dir3"))
		LDAz(d1 + 0);
		CLC();
		ADCi(1);
		STAz(d1 + 0);
		LDAz(d1 + 1);
		ADCi(0);
		STAz(d1 + 1);
		JMP(L("nodir"))

		LPC("dir3")
		CMPi('v');
		BNE(L("dir4"))
		LDAz(d1 + 0);
		CLC();
		ADCi(40);
		STAz(d1 + 0);
		LDAz(d1 + 1);
		ADCi(0);
		STAz(d1 + 1);
		JMP(L("nodir"))

		LPC("dir4")
		CMPi('<');
		BNE(L("dirend"))
		LDAz(d1 + 0);
		SEC();
		SBCi(1);
		STAz(d1 + 0);
		LDAz(d1 + 1);
		SBCi(0);
		STAz(d1 + 1);

		LPC("dirend")
		CMPi('X');
		BNE(L("nodir"))

		LDAi(0)
		STA(d1 + 1)


		LPC("nodir");

		LPC("skipdrones")
		RTS();

	if (g_pass == 0)
		spiral::make();


	LPC("dronespeedtab");

	for(i = 0; i < 0x20; i++)
	{
		int val = 1 + pow((float)i / (float)0x1F, 2.0f) * 0x20;

		if (val < 1)
			val = 1;
		else if (val > 0x20)
			val = 0x20;
//		printf("%i = %i\n", i, val);
		B(val);
	}

	LPC("dronemap");
	for(i = 0; i < 40*25; i++)
	{
		B(spiral::tab[i]);
	}

	LPC("dronestarts_l");
	for(i = 0; i < spiral::startpointcount; i++)
	{
		value_t sp = spiral::startpoints[i] + L("dronemap");
		B(LB(sp));
	}
	B(0);
	LPC("dronestarts_h");
	for(i = 0; i < spiral::startpointcount; i++)
	{
		value_t sp = spiral::startpoints[i] + L("dronemap");
		B(HB(sp));
	}

	B(0);

	int cl1[] = {0xE, 0xe, 0xe, 4, 6,  0, 0};

	LPC("cta1");
	write_colortab(cl1, 7, 0);

	int cl2[] = {0x4, 0x4, 0x4, 6, 6,  0, 0};

	LPC("cta2");
	write_colortab(cl2, 7, 0);

	int cl3[] = {0x6, 0xc, 4, 4, 4, 4, 4, 4, 4, 4};

	LPC("cta3");
	write_colortab(cl3, 10, 0);
}

void genis()
{
	msdoseffu();
}

int main()
{
	assemble(genis);

	list<segment_c *> segs;
	segs.push_back(&actualprg);
	make_prg("msdos.prg", msdos_start, segs);
}
