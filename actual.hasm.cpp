#include <debugf.h>
#include <hashembler.h>

#include <math.h>
#include "lib.h"

#include "megapetscii/bundledata.h"

#include "krill.h"

using namespace hashembler;

segment_asm_c actualprg;

void tables();

void mainthread()
{
	int x;


/*

	THE MAIN LOOP

*/
	LPC("drawcolumns");

	LDAz(dbg_reset_flag)
	BEQ(L("no_reset"))
	JMP(actual_start);

	LPC("no_reset");


	JSR(L("iterate_refresher"))

	// loader
	{

#if 1

#ifdef MEGAPETSCII_DEBUG_SHOW_FILE
		int i;
		for (i = 0; i < 6; i++)
		{
			LDA(L("newfn") + i)
			STA(0x400 + DEBUG_POS*40 + i)
			INC(0xd800 + DEBUG_POS*40 + i)
		}
#endif

	// check if we should load
	LDX(L("newfn"))
	BNE(L("new_stuff_to_load"))
	JMP(L("skipped_loading"))

	LPC("new_stuff_to_load")

/*
	LDXz(loading_bundle);
	CPXi(0x21);
	BNE(L("avoidthis"));
	LDXi(0);
	STX(L("newfn"));
	JMP(L("skipped_loading"))
	LPC("avoidthis");
*/

	LDAi(0x35);
	STAz(0x01);

	LDXi(LB(L("newfn")));
	LDYi(HB(L("newfn")));

	PRINTPOS("loadercall")
	CLC();
	//SEI();
	JSR(krill_load_packed)
//	JSR(krill_load)
	BCC(L("segmentloaded"))
//	SEI();
//	JMP(L("loaderror"))
	LPC("segmentloaded")

	// re-enable irq - dunno if necessary ????
	CLI();
	MOV8i(0xD01A, 0x01);

	LDAi(0x35);
	STAz(0x01);

	LDYi(0xFF);
	STYz(loading_bundle); // write 0xff

	LDAi(0)
	STA(L("newfn"))

	// loading done, bunch prepared for consumption and loader marked as free
#endif
	}
	LPC("skipped_loading")
	JMP(L("drawcolumns"))

/*

	COLUMN REFRESHER
	this routine should be be re-entrant !
	( at least the part where column number is updated )

*/
	LPC("iterate_refresher");
	PRINTPOS("iterate_refresher")

	PHP();
	SEI();

	// find next column to refresh + randomize
	LDYz(refresh_cur_column)
	LDXy(L("randomtab"))
	STXz(refresh_cur_column);
	//PLP(); // don't enable both !!
	//CLI();

#ifdef BORDER_DEBUG
	STY(0xd020);
#endif
	// compare column buffers - is update necessary? - not optimized!
	LDAx(colbuf_l)
	CMPx(colbuf_cur_l)
	BNE(L("handle_col"));
	LDAx(colbuf_h)
	CMPx(colbuf_cur_h)
	BNE(L("handle_col"));

	// this column is fine, return
	PLP();
	RTS();

	// -- actual column handler --

	// load column number and do some shift magic, and prepare shifted column for later use
	LPC("handle_col");

	LDAx(colbuf_h)

	// lda asl sta rol asl rol
	LDYx(colbuf_l)
	STYz(tmp_col_l)

	#define COLUMNS_PER_BUNDLE (128)
	ASLz(tmp_col_l);
	ROLa();
//	#define COLUMNS_PER_BUNDLE (64)
//	ASLz(tmp_col_l);
//	ROLa();

	STAz(tmp_col_h)

	LDYx(colbuf_l);
	TAX();

	 // update data base pointer
	CLC();
	LDAx(L("bundletab_lb"));
	ADCy(L("columntab_lb"))
	STAz(zpbase);
	LDAx(L("bundletab_hb"));
	ADCy(L("columntab_hb"))
	STAz(zpbase + 1);

	//LDXz(refresh_cur_column)	

	// this is bundle loaded?
	LDYi(25+25);
	LDAizy(zpbase);
//	CMPx(colbuf_h)
	CMPz(tmp_col_h)
	BEQ(L("noneedtoload"));

//	LDAi(0xFF);
//	STAx(colbuf_cur_h)
//	STAx(colbuf_cur_l)

//	JSR(L("blankcolumn"))

	// don't init new loading if there's active one / pending
	LDA(L("newfn"))
	BEQ(L("ok_to_load"))
	PLP();
	RTS();

	// init loading 
	LPC("ok_to_load")

//	STXz(tmp_x);

//	LDAx(colbuf_h)
	LDAz(tmp_col_h)
	STAz(loading_bundle)
	// convert chunk number to hex
	LSRa();
	LSRa();
	LSRa();
	LSRa();
	ANDi(0x0F);
	TAX();
	LDAx(L("hextab"));
	STA(L("newfn") + 0);

	LDAz(tmp_col_h)
	ANDi(0x0F);
	TAX();
	LDAx(L("hextab"));
	STA(L("newfn") + 1);

//	LDXz(tmp_x);

	PLP();
	RTS();


	// data for hex conversion & loading
	LPC("hextab");
	int i;
	const char *hex = "0123456789ABCDEF";
	for (i = 0; i < 16; i++)
	{
		B(hex[i]);
	}

	LPC("newfn")
	SEG->add_string("\0000.PET.LZ", 10);



	// so column in memory - just update !

	LPC("noneedtoload");

	// update buffers
	LDXz(refresh_cur_column)

	LDAx(colbuf_h)
	STAx(colbuf_cur_h)
	LDAx(colbuf_l)
	STAx(colbuf_cur_l)
	//TAY();

//  don't update actual gfx, just drop out
//	PLP();
//	RTS();

	{
		int i;

		LDYi(0);
		for (i = 0; i <	 25; i++)
		{
			LDAizy(zpbase);

			if (i >= MEGAPETSCII_DEBUG_AREA)
				STAx(0x400 + i * 40)
			INY();
		}

		for (i = 0; i < 25; i++)
		{
			LDAizy(zpbase);
			if (i >= MEGAPETSCII_DEBUG_AREA)
				STAx(0xd800 + i * 40)
			if (i < 24)
				INY();
		}
	}

	LDAx(text_colbuf)
	BNE(L("draw_char_here"))
	PLP();
	RTS();
	LPC("draw_char_here")

	LDAz(text_pos_screen + 0)
	STA(L("screen_write_op") + 1)
	STA(L("color_write_op") + 1)

	CLC();
	LDAz(text_pos_screen + 1)
	ADCi(0x04); // 0x0400 is screen mem start! (this should clear C always)
	STA(L("screen_write_op") + 2)
	ADCi(0xd8 - 0x04); // bit more! (C should be 0)
	STA(L("color_write_op") + 2)


	LDAx(text_colbuf)
	LPC("screen_write_op")
	STAx(0x400 + 4 * 40)

	LDAz(text_color);
	LPC("color_write_op")
	STAx(0xD800 + 4 * 40)


	PLP();
	RTS();

	LPC("blankcolumn");

/*
	//                       0123456701234567012345678
	const char *emptytext = "     LOADING MEGAPETSCII ";
	for (i = 0; i < 25; i++)
	{
		LDAi(emptytext[i] - 'A' + 1)
		if (i >= MEGAPETSCII_DEBUG_AREA)
			STAx(0x400 + i * 40)
	}
*/
	LDAi(0);
	for (i = 0; i < 25; i++)
	{
		if (i >= MEGAPETSCII_DEBUG_AREA)
			STAx(0xd800 + i * 40)
	}

	RTS();
}

void mov16_z(value_t outaddr, value_t inaddr)
{
	LDA(inaddr+0)
	STA(outaddr+0)
	LDA(inaddr+1)
	STA(outaddr+1)
}

void div16_fast_z(value_t addr)
{
	CMPi(0x80);
	ROR(addr + 1);
	ROR(addr + 0);
}

void sub16_z(value_t outaddr, value_t addr1, value_t addr2)
{
	LDA(addr1 + 0);
	SEC();
	SBC(addr2 + 0);
	STA(outaddr + 0);
	LDA(addr1 + 1);
	SBC(addr2 + 1);
	STA(outaddr + 1);
}

void add16_z(value_t outaddr, value_t addr1, value_t addr2)
{
	LDA(addr1 + 0);
	CLC();
	ADC(addr2 + 0);
	STA(outaddr + 0);
	LDA(addr1 + 1);
	ADC(addr2 + 1);
	STA(outaddr + 1);
}

void twister_spring_init()
{
	LDAi(0);
	STAz(st4 + 0)
	STAz(st4 + 1)
	STAz(twister_vel + 0)
	STAz(twister_vel + 1)
	STAz(twister_dist + 0)
	STAz(twister_dist + 1)

	MOV16i(st4, 0x2100);
	MOV16i(twister_target, 0);
}

void twister_spring(value_t posaddr)
{
/*	LDAz(cumctx_head + 1)
	STAz(twister_target + 0)
	LDAz(cumctx_head + 0)
	ANDi(0x1f);
	SEC();
	SBCi(0x10);
	STAz(twister_target + 1)
*/
	// dampen velocity
	mov16_z(twister_temp, twister_vel);
	LDAz(twister_temp + 1)
	div16_fast_z(twister_temp);
	div16_fast_z(twister_temp);
	div16_fast_z(twister_temp);
	div16_fast_z(twister_temp);
//	div16_fast_z(twister_temp);
//	div16_fast_z(twister_temp);
	sub16_z(twister_vel, twister_vel, twister_temp);

	// integrate position
	add16_z(posaddr, posaddr, twister_vel);

	//MOV16i(twister_vel, 0);

	// calculate distance to target
	sub16_z(twister_dist, twister_target, posaddr);
	LDAz(twister_dist + 1)
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);
	div16_fast_z(twister_dist);

	// add to velocity
	add16_z(twister_vel, twister_vel, twister_dist);
}

void irqroutine()
{
	LPC("irqrut");
	PRINTPOS("irqrut");
	LSR(0xd019);

#ifdef BORDER_DEBUG
	LDAi(1)
	STA(0xd020);
#endif

	LDAi(0x7f);
	STA(0xdc0d);
	STA(0xdd0d);
	LDA(0xdc0d);
	LDA(0xdd0d);

	ADD16z_i(framecounter, 1);

	// store bankswitching state to stack !
	LDAz(0x01);
	PHA();

	LDAi(0x35);
	STAz(0x01);
	JSR(music_play);

#ifdef MANUAL_JOYSTICK_CONTROL
	LDA(0xDC00);
	ANDi(0x10);
	BEQ(L("skip_cumvm"))
	JSR(cumvm_play);
	LPC("skip_cumvm")
#else
	JSR(cumvm_play);
#endif

#ifdef BORDER_DEBUG
	INC(0xd020);
#endif

/*
	LDXi(0x0)
	STXz(scrollpos_a + 0)
	STXz(scrollpos_a + 1)
	STXz(scrollpos_a + 2)
*/

#ifdef MANUAL_JOYSTICK_CONTROL

	LDAz(lastpressed);
	ANDi(0x10);
	BEQ(L("no_clear_accel"))

	LDXi(0x0)
	STXz(scrollpos_a + 0)
	STXz(scrollpos_a + 1)
	STXz(scrollpos_a + 2)

	LPC("no_clear_accel");

	LDA(0xDC00);
	ANDi(0x1F)
	EORi(0x1F)
	STAz(lastpressed);
	TAY();

	ANDi(0x10);
	BEQ(L("nofire"))

	TYA();
	ANDi(0x0F);
	TAY();

#if 0
	CPYi(0x10);
	BNE(L("nosongchange"))
	LDAi(0x02);
	//JSR(0xE000);
	//JSR(0xFFEE);
	JMP(L("noright"))
#endif

	LPC("nosongchange");



#if 0
	CPYi(0x01);
	BNE(L("nodown"))
	DECz(vsp_pos)
	LPC("nodown");
#endif


	CPYi(0x08);
	BNE(L("noleft"))
	LDXi(0x20)
	STXz(scrollpos_a + 0)
	LPC("noleft");

	CPYi(0x04);
	BNE(L("noright"))
	LDXi(0xE0)
	STXz(scrollpos_a + 0)
	LDXi(0xFF)
	STXz(scrollpos_a + 1)
	LDXi(0xFF)
	STXz(scrollpos_a + 2)
	LPC("noright");

	LPC("nofire");

#endif

//	PLA();
//	STAz(0x01);

	int i;

// vsp
#if 0

// stable raster !

	LDAi(0x11)
	STA(0xd020)

	LDAi(0x1C)
	STA(0xd011)

	LDAi(0x32);
	LPC("wait_for_screen_begin")
	CMP(0xd012);
	BNE(L("wait_for_screen_begin"))

	LDA(0xdd04)				// kestää 4 sykliä
	EORi(0xff);
	CLC();
	ADCz(vsp_pos);
	LSRa();					// jako / 2:lla, eka bitti = C
	BCC(L("onecycle")) 		// 2 sykliä normisti, jos C=1 nii 3
	LPC("onecycle");

	// bccmod = 0x20 -> LSR -> 0x40 -> ADC 0x15 -> 0x2B -> EOR 0x3f -> 0x14
	// 0x14 -> EOR 0x3f -> 0x2b -> ADC 0x15 -> 0x40 -> LSR -> 0x20

	// 0x14 -> ADC 0xAB -> 0xBF ->EOF 0xFF -> 0x40 -> LSR -> 0x20

	// 08 - LSR - 10 - EOR FF - EF - ADC C9 - 26
	// 26 - ADC C9 - EF - EOR FF - 10 - LSR -> 08

	// 02 - LSR - 04 - EOR FF - FB - ADC D5 - 26
	// 26 - ADC D5 - FB - EOR FF - 04 - LSR -> 02


	STA(L("bccmod") + 1) 	// 4 sykliä
	LPC("bccmod")
	BPL(L("bccend"));		// hyppää eteenpäin cia arvo / 2 byteä, eli int(cia/2)*2 sykliä
	LPC("bccend")

	PRINTPOS("nopshit")

/*
	0xA1
	0xC9

*/

	//for (i = 0; i < 0x64; i++)
//	for (i = 0; i < 0x5E; i++)
	for (i = 0; i < 0x35; i++)
		B(0xEA);
	PRINTPOS("nopshit_end")
	LDAz(0)

	LDAi(0x1B);
	STA(0xd011)
//	LDAi(0x12);
//	STA(0xd020);


	INC(0xd020);

#endif

// debug values view
#ifdef DEBUG_POS

	LDAz(loading_bundle)
	LDYi(8);
	JSR(L("drawhex"))


	LDAz(scrollpos+2);
	LDYi(14);
	JSR(L("drawhex"))
	LDAz(scrollpos+1);
	LDYi(16);
	JSR(L("drawhex"))
	LDAz(scrollpos+0);
	LDYi(18);
	JSR(L("drawhex"))

	LDAz(framecounter+1);
	LDYi(22);
	JSR(L("drawhex"))
	LDAz(framecounter+0);
	LDYi(24);
	JSR(L("drawhex"))

	LDAz(cumctx_head+1);
	LDYi(31);
	JSR(L("drawhex"))
	LDAz(cumctx_head+0);
	LDYi(33);
	JSR(L("drawhex"))

	LDAz(cumctx_nextthink+1);
	LDYi(36);
	JSR(L("drawhex"))
	LDAz(cumctx_nextthink+0);
	LDYi(38);
	JSR(L("drawhex"))


	JMP(L("hextabover"))

	LPC("drawhex")
	PHA();
	LSRa();
	LSRa();
	LSRa();
	LSRa();
	ANDi(0x0F);
	TAX();
	LDAx(L("pethextab"));
	STAy(0x400 + 40 * DEBUG_POS);

	PLA();
	ANDi(0x0F);
	TAX();
	LDAx(L("pethextab"));
	STAy(0x400 + 40 * DEBUG_POS + 1);
	RTS();

	// data for hex conversion & loading
	LPC("pethextab");
	for (i = 0; i < 16; i++)
	{
		int as;
		if (i <= 9)
			as = 0x30+i;
		else
			as = i - 10 + 1;
		B(as);
	}
	LPC("hextabover")

#endif

#ifdef BORDER_DEBUG
	INC(0xd020);
#endif

	int x;

// inertial scrolling
#if 1
	CLC();
	LDAz(scrollpos_v + 0)
	ADCz(scrollpos_a + 0)
	STAz(scrollpos_v + 0)
	LDAz(scrollpos_v + 1)
	ADCz(scrollpos_a + 1)
	STAz(scrollpos_v + 1)
	LDAz(scrollpos_v + 2)
	ADCz(scrollpos_a + 2)
	STAz(scrollpos_v + 2)

	CLC();
	LDAz(scrollpos + 0)
	ADCz(scrollpos_v + 0)
	STAz(scrollpos + 0)
	LDAz(scrollpos + 1)
	ADCz(scrollpos_v + 1)
	STAz(scrollpos + 1)
	LDAz(scrollpos + 2)
	ADCz(scrollpos_v + 2)
	STAz(scrollpos + 2)
#endif

// vsp-less fullscreen column update
#if 0
	for (x = 0; x < 40; x++)
	{
		LDAz(scrollpos + 1)
		CLC();
		ADCi(x);
		STA(colbuf_l + x)
		LDAz(scrollpos + 2)
		ADCi(0);
		STA(colbuf_h + x)
	}
#endif

//	JMP(L("scroll_twistor"));
	LDAz(scrollmode)
	ANDi(0x0F);
//	LDAi(2);
	BEQ(L("scroll_normal"));

	CMPi(2);
	BEQ(L("goto_scroll_endless"))
	JMP(L("scroll_twistor"));

	LPC("goto_scroll_endless")
	JMP(L("scroll_endless"));

// bit faster vsp-less fullscreen column update
#if 1
	LPC("scroll_normal")
	LDA(scrollpos + 1)
	STA(colbuf_l + 0)
	LDA(scrollpos + 2)
	STA(colbuf_h + 0)
	CLC();
	for (x = 1; x < 40; x++)
	{
		LDA(colbuf_l + x - 1);
		ADCi(1);
		STA(colbuf_l + x);
		LDA(colbuf_h + x - 1);
		ADCi(0);
		STA(colbuf_h + x);
	}
	JMP(L("scroll_decided_and_done"));
#endif

// twistah!
#if 1
	PRINTPOS("twistor")
	LPC("scroll_twistor")

	ADD16z_i(st1, 0x465);

	ADD16z_i(st2, 0xFE69);

	twister_spring(twister_pos);

	mov16_z(st4, twister_pos);


	LDXz(st2 + 1)
	LDAz(st1 + 0)
//	ADCz(twister_pos + 0)
	//ADCx(L("sinus3"))
	STAz(st3 + 0)
	LDAz(st1 + 1)
//	ADCz(twister_pos + 1)
	//ADCx(L("sinus4"))
	STAz(st3 + 1)

	for (x = 20; x < 40; x++)
	{
		if ((x % 5) == 3)
		{
			value_t incval = (x-20) * 0xf8;
			CLC();
			LDAz(st3+0)
			ADCi(LB(incval));
			STAz(st3+0)
			LDAz(st3+1)
			ADCi(HB(incval));
			STAz(st3+1)
		}
		CLC();
		LDAz(st3+0)
		ADCz(st4 + 0)
		STAz(st3+0)
		LDAz(st3+1)
		ADCz(st4 + 1)
		STAz(st3+1)
		TAX();

		//LDX(colbuf_l + x)

		LDAx(L("twistertab_l"))
		STA(colbuf_l + x)
		LDAx(L("twistertab_h"))
		//LDAi(0);
		STA(colbuf_h + x)
	}

	LDXz(st2 + 1)
	LDAz(st1 + 0)
	//ADCx(L("sinus3"))
	STAz(st3 + 0)
	LDAz(st1 + 1)
	//ADCx(L("sinus4"))
	STAz(st3 + 1)

	for (x = 19; x >= 0; x--)
	{
		TAX();
		//LDX(colbuf_l + x)

		LDAx(L("twistertab_l"))
		STA(colbuf_l + x)
		LDAx(L("twistertab_h"))
		//LDAi(0);
		STA(colbuf_h + x)

		if ((x % 5) == 3)
		{
			value_t incval = (19-x) * 0xf8;
			SEC();
			LDAz(st3+0)
			SBCi(LB(incval));
			STAz(st3+0)
			LDAz(st3+1)
			SBCi(HB(incval));
			STAz(st3+1)
		}

		if (x > 0)
		{
			SEC();
			LDAz(st3+0)
			SBCz(st4 + 0)
			STAz(st3+0)
			LDAz(st3+1)
			SBCz(st4 + 1)
			STAz(st3+1)
		}
	}

		//STA(colbuf_l + x);
	JMP(L("scroll_decided_and_done"));
#endif

	LPC("scroll_endless");

#if 0
	LDAz(st1+0)
	CLC();
	ADCz(0xE0);
	STAz(st1+0);
	LDAz(st1+1);
	ADCi(0);
	CMPi(4*40-1);
	BNE(L("noendlessreset"))
	LDAi(1*40-1);
	LPC("noendlessreset");
	STAz(st1+1);
	TAY();
#endif

	value_t endcolsub = 234 * 40 - 40;
	LDAz(scrollpos+1)
	SEC();
	SBCi(LB(endcolsub))

	TAY();

	CMPi(3*40+40);
	BCC(L("noendlessreset"))
	CLC();
	ADCi((0x100 - (3*40) + LB(endcolsub)) & 0xFF);
	STAz(scrollpos+1)
//	LDYi(((1*40) + LB(endcolsub)) & 0xFF);
//	STYz(scrollpos+1)
	LPC("noendlessreset");

	CMPi(1*40);
	BCS(L("noendlessreset2"))
//	LDYi(((4*40-1) + LB(endcolsub)) & 0xFF);
//	STYz(scrollpos+1)
	CLC();
	ADCi((0x100 + (3*40) + LB(endcolsub)) & 0xFF);
	STAz(scrollpos+1)

	LPC("noendlessreset2");


	LDXi(39);
	LPC("endless_render");
	LDAy(L("endlesscols_l"))
	STAx(colbuf_l);
	LDAy(L("endlesscols_h"))
	STAx(colbuf_h);
	DEY();
	DEX();
	BPL(L("endless_render"));
	JMP(L("scroll_decided_and_done"));

	// 0  1  2  3  4
	// 012012012012

	int endlesscols[4*40];
	LPC("endlesscols_l")
	for (x = 0; x < 4*40; x++)
	{
		endlesscols[x] = 234 * 40 + (x % (40*3));
		B(LB(endlesscols[x]));
	}
	LPC("endlesscols_h")
	for (x = 0; x < 4*40; x++)
	{
		B(HB(endlesscols[x]));
	}




	LPC("scroll_decided_and_done");


// vsp-ready column update
#if 0
	// calculate (scrollpos % 40) and update scrolltmp
	// (scrolltmp subtracting might be avoided - just copy scrollpos and increment it in "rightpart" loop
	LDXz(scrollpos + 2)
	LDAx(L("screenmodtab1"))
	CLC();
	LDXz(scrollpos + 1)
	ADCx(L("screenmodtab2"))
	TAX();

	LDAz(scrollpos + 1)
	SEC();
	SBCx(L("screenmodtab2"))
	STAz(scrolltmp + 0)
	LDAz(scrollpos + 2)
	SBCi(0);
	STAz(scrolltmp + 1)

	LDAi(0x30)
	CLC();
	ADCx(L("screenmodtab2"))
	LDAz(0x30);
	//STAz(vsp_pos)


	LDAx(L("screenmodtab2"))
	// x has (scrollpos % 40) !

	LDXi(0);
	CMPi(0);
	BEQ(L("rightpart"))

#if 1
	// this updates the next screen - left half
	STA(L("leftpart_mod") + 1)

	LDAz(scrolltmp + 0)
	CLC();
	ADCi(40);
	STAz(scrolltmp2 + 0)
	LDAz(scrolltmp + 1)
	ADCi(0);
	STAz(scrolltmp2 + 1)

	LPC("leftpart")
	TXA();
	ADCz(scrolltmp2 + 0)
	STAx(colbuf_l)
	LDAi(0);
	ADCz(scrolltmp2 + 1)
	STAx(colbuf_h)

	INX();
	LPC("leftpart_mod")
	CPXi(40);
	BNE(L("leftpart"))
#endif
	// and this updates the right half of the screen

	LPC("rightpart")
	TXA();
	CLC();
	ADCz(scrolltmp + 0)
	STAx(colbuf_l)
	LDAi(0);
	ADCz(scrolltmp + 1)
	STAx(colbuf_h)
	INX();
	CPXi(40);
	BNE(L("rightpart"))




#endif

#if 0
	LDXz(scrollpos + 2)
	LDAx(L("screenmodtab1"))
	CLC();
	ADCz(scrollpos + 1)
	TAX();

	LDAz(scrollpos + 1)
	SEC();
	SBCx(L("screenmodtab2"))
	STAz(scrolltmp + 0)
	LDAz(scrollpos + 2)
	SBCi(0);
	STAz(scrolltmp + 1)
	for (x = 0; x < 40; x++)
	{
		LDAz(scrolltmp + 0)
		CLC();
		ADCi(x);
		STA(colbuf_l + x)
		LDAz(scrolltmp + 1)
		ADCi(0);
		STA(colbuf_h + x)
	}
#endif

#if 0
	LDXz(scrollpos + 2)
	LDAx(L("screenmodtab1"))
	CLC();
	ADCz(scrollpos + 1)
	TAX();
	LDAx(L("screenmodtab2"))
	TAX();

	LDAz(scrollpos + 1)
	STAx(colbuf_l)
	LDAz(scrollpos + 2)
	STAx(colbuf_h)
#endif

// fine scroll
#if 0
	LDA(0xDC00);
	ANDi(0x02);
	BNE(L("nosmoothscroll"))
	LDAz(scrollpos + 0)
	ROLa();
	ROLa();
	ROLa();
	ROLa();
	ANDi(0x07);
	EORi(0x0F);
//	STAz(d016_val)
	STA(0xd016);

	LPC("nosmoothscroll")
#endif

	LDAi(0xc8)
	STA(0xd016);

	LDAz(scrollmode)
	ANDi(0x80);
	BNE(L("sprite_display"))

	JMP(L("no_sprite_display"))

	LPC("sprite_display")

	LPC("car_move_mod")
	LDXi(0x00);

	INC(L("car_move_mod") + 1)
	BNE(L("car_moving"))
	DEC(L("car_move_mod") + 1)
	LPC("car_moving")

	LDYz(framecounter + 0)
	LDAx(L("carpos_l"))
	ADCy(L("sinus3"))
	STAz(sprite_x + 0)
	LDAx(L("carpos_h"))
	ADCi(0);
	STAz(sprite_x + 1)

	LDA(framecounter + 0)
	ANDi(0x01);
	ADCi(0x83);
	STAz(sprite_y);

	LDAz(sprite_x + 0)
	PHA();
	LDAz(sprite_x + 1)
	PHA();
	LDAz(sprite_y)
	PHA();
	LDAi(0xC0)
	PHA();
	LDAi(0x07)
	PHA();

	LDAz(sprite_x + 0)
	PHA();
	LDAz(sprite_x + 1)
	PHA();
	LDAz(sprite_y)
	PHA();
	LDAi(0xC1)
	PHA();
	LDAi(0x0B)
	PHA();

	LDAz(sprite_x + 0)
	PHA();
	LDAz(sprite_x + 1)
	PHA();
	LDAz(sprite_y)
	PHA();
	LDAi(0xC2)
	PHA();
	LDAi(0x02)
	PHA();

	LDAz(sprite_x + 0)
	CLC();
	ADCi(24);
	STAz(st1 + 0)
	LDAz(sprite_x + 1)
	ADCi(0);
	STAz(st1 + 1)
	LDAz(sprite_y)
	STAz(st2 + 0)

	LDAz(st1 + 0)
	PHA();
	LDAz(st1 + 1)
	PHA();
	LDAz(st2)
	PHA();
	LDAi(0xC3)
	PHA();
	LDAi(0x07)
	PHA();

	LDAz(st1 + 0)
	PHA();
	LDAz(st1 + 1)
	PHA();
	LDAz(st2)
	PHA();
	LDAi(0xC4)
	PHA();
	LDAi(0x02)
	PHA();

	LDAz(st1 + 0)
	PHA();
	LDAz(st1 + 1)
	PHA();
	LDAz(st2)
	PHA();
	LDAi(0xC5)
	PHA();
	LDAi(0x0B)
	PHA();

	// reflection

	LDAz(sprite_x + 0)
	CLC();
	ADCi(24);
	STAz(st1 + 0)
	LDAz(sprite_x + 1)
	ADCi(0);
	STAz(st1 + 1)
	LDAz(sprite_y)
	ADCi(0x3A);
	STAz(st2 + 0)

	LDAz(sprite_x + 0)
	PHA();
	LDAz(sprite_x + 1)
	PHA();
	LDAz(st2)
	PHA();
	LDAi(0xC6)
	PHA();
	LDAi(0x0B)
	PHA();

	LDAz(st1 + 0)
	PHA();
	LDAz(st1 + 1)
	PHA();
	LDAz(st2)
	PHA();
	LDAi(0xC7)
	PHA();
	LDAi(0x0B)
	PHA();

#if 0
	for (i = 0; i < 2; i++)
	{
		int pos = 0x50 + i *0x19;
		// push sprite

		LDAi(LB(pos))
		PHA();
		LDAi(HB(pos))
		PHA();
		LDAi((int)(0x40 + sin(i) * 10))
		PHA();
		LDAi(0x80 + i)
		PHA();
		LDAi(0x01+i);
		PHA();
	}


#endif
	// all sprite flags off
	LDAi(0)
	STAz(gh_d015);
	STAz(gh_d010);

	LDAi(0xFC);
	STA(0xd01B);

	LDAi(0x01);
	LDXi(0x00);
	LDYi(0x00);
	STAz(sprite_iterator_temp)
	PRINTPOS("xz")

	LPC("sprite_iterator_loop")

	PLA();
	STAx(0xD027);

	PLA();
	STAx(0x7F8);

	PLA();
	STAy(0xd001);

	PLA();

	PHA();
	ANDi(0xFE);
	BNE(L("sprite_is_off"))
	LDA(sprite_iterator_temp);
	ORAz(gh_d015);
	STAz(gh_d015);
	LPC("sprite_is_off");

	PLA();
	ANDi(0x01);
	BEQ(L("no_x_nine"))
	LDA(sprite_iterator_temp);
	ORAz(gh_d010);
	STAz(gh_d010);
	LPC("no_x_nine");

	PLA();
	STAy(0xD000);


	INX();
	INY();
	INY();
	ASLz(sprite_iterator_temp);
	BNE(L("sprite_iterator_loop"))

//	B(2);

	LDAz(gh_d010);
	STA(0xd010);
	LDAz(gh_d015);
	STA(0xd015);

	LPC("no_sprite_display")


/*
	LDAi(0x80)
	STA(0x7F8);
	LDAi(1)
	STA(0xD027);
	LDAi(0x50)
	STA(0xD001);
	LDAi(0x40)
	STA(0xD000);
	LDAi(0)
	STA(0xD010);
	LDAi(0xFF)
	STA(0xD015);
*/
	// JMP(L("loading_not_active"))

	// if loading is active - update some columns!
	LDX(L("newfn"))
	BEQ(L("loading_not_active"))

	// TODO: we should only run this for 40 iterations!
	LPC("irq_loading_loop")
	JSR(L("iterate_refresher"))
	LDA(0xd012)
	CMPi(0xE0)
	BCC(L("irq_loading_loop"))

	LPC("loading_not_active")


	PLA();
	STAz(0x01);

#ifdef BORDER_DEBUG
	LDAi(0);
	STA(0xd020)
#endif

	PRINTPOS("return from irq")
	RTS();


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
}

void start_stabilizer()
{
	PRINTPOS("stabilization begins")
	// cia interval
	LDAi(62); STA(0xdd04); LDAi(00); STA(0xdd05);

	// wait for upper part of screen
	LDA(0xD011); ANDi(0x7F); LPC("wait0") CMP(0xd011); BNE(L("wait0"))

	// wait for lower part of screen
	LDA(0xD011); ORAi(0x80); LPC("wait1") CMP(0xd011); BNE(L("wait1"))

	// wait for line
	LDAi(0x10); LPC("wait2"); CMP(0xd012); BNE(L("wait2"));

	//INC(0xd020);

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
		DEX();  // 2
		BNE(L("cyclewaster")); // 3 (-1 on last)
	JMP(L("syncloop")) // 3

	LPC("now_out")
	// here we are. scanline start + 3 + 3 cycles.

	LDXi(3)
	LPC("ciaoffsetter")
	DEX();
	BNE(L("ciaoffsetter"))

	LDYi(0x91);
	STY(0xdd0e)

	PRINTPOS("cia started")

	//MOV8i(vsp_pos, 0x55);
}

void actualprog()
{
	CTX("actualprog")

	actualprg.begin(actual_start);
	set_segment(&actualprg);

	SEI();
	MOV16i(0x0314, L("irqrut_kernal"));
	MOV16i(0xFFFE, L("irqrut_direct"));
	MOV8i(0xD01A, 0x01);
	MOV8i(0xD012, 0x20);
	MOV8i(0xD011, 0x1B);

	LDAi(0x7f);
	STA(0xdc0d);
	STA(0xdd0d);
	LDA(0xdc0d);
	LDA(0xdd0d);


	LDAi(0)
	STAz(dbg_reset_flag)

#if 0
	start_stabilizer();
#endif

	LDAi(0x35);
	STAz(0x01);

//	LDAi(0x01);
//	JSR(music_init);

	PRINTPOS("call to cum")
	JSR(cumvm_init);

	LDAi(0)
	STA(0xd020);
	STA(0xd021);

	JMP(L("ax"));

	LPC("loaderror");
	STX(0xd020);
	STA(0xd020);
	LDXi(0x02);
	JMP(L("loaderror"));

	irqroutine();

	LPC("ax");
	LDAi(0);
	STAz(scrollpos + 0)
	STAz(scrollpos + 1)
	STAz(scrollpos + 2)
	STAz(scrollpos_v + 0)
	STAz(scrollpos_v + 1)
	STAz(scrollpos_v + 2)
	STAz(scrollpos_a + 0)
	STAz(scrollpos_a + 1)
	STAz(scrollpos_a + 2)

	LDXi(39);
	LDAi(0xFF);
	LPC("columnclear")
	STAx(colbuf_l)
	STAx(colbuf_h)
	STAx(colbuf_cur_l)
	STAx(colbuf_cur_h)

	STAx(text_colbuf)

	int i;
	for (i = 0; i < MEGAPETSCII_DEBUG_AREA; i++)
	{
		STAx(0xd800 + 40 * i)
	}

	DEX();
	BPL(L("columnclear"));

	LDXi(39);
	LDAi(0);
	LPC("columnclear2")
	STAx(text_colbuf)
	DEX();
	BPL(L("columnclear2"));


	value_t testpos = 2 * 40;
	LDAi(LB(testpos));
	STAz(text_pos_screen + 0);
	LDAi(HB(testpos));
	STAz(text_pos_screen + 1);
	LDAi(1);
	STAz(text_color);
/*
	LDAi('6')
	STA(text_colbuf + 10)
	STA(text_colbuf + 11)
	STA(text_colbuf + 12)
*/

	LDAi(0);
	STAz(refresh_cur_column);
	STAz(scrollmode);

	twister_spring_init();


	CLI();

#if 1
	LDXi(bundle_addr_count - 1);
	LPC("memclearloop1")
	LDYi(0);
	LPC("memclearloop2")

	CLC();
	LDAx(L("bundletab_lb"));
	ADCy(L("columntab_lb"))
	STAz(zpbase);
	LDAx(L("bundletab_hb"));
	ADCy(L("columntab_hb"))
	STAz(zpbase + 1);

	TYA();
	LDYi(25+25);
	STAizy(zpbase);
	TAY();


	INY();
	BNE(L("memclearloop2"))
	DEX();
	BNE(L("memclearloop1"))
#endif

	mainthread();

	tables();
}

void tables()
{
	int x;
	int i;
	PAGE
	PRINTPOS("sinus tables");
	int sinus[256];
	LPC("sinus1")
	for (x = 0; x < 256; x++)
	{
		//float move = 210.f;
		float move = 300.f;
		move += sin(x * (M_PI / 128.f) + 0.4f) * 4626.f;
		//move += sin(x * (M_PI / 64.f) + 5.7f) * 500.f;
//		move += sin(x * (M_PI / 32.f) + 4.9f) * 40.f;
		sinus[x] = (0x1000000 + (int)move) & 0xFFFFFF;
		int as = (sinus[x] >> 0) & 255;
		B(as)
	}
	LPC("sinus2")
	for (x = 0; x < 256; x++)
	{
		int as = (sinus[x] >> 8) & 255;
		B(as)
	}

	LPC("sinus3")
	for (x = 0; x < 256; x++)
	{
		float move = 0x15;
		move += sin(x * (M_PI / 128.f) + 0.4f) * 10.f;
		move += sin(x * (M_PI / 64.f) + 5.7f) * 5.f;
//		move += sin(x * (M_PI / 32.f) + 4.9f) * 40.f;
		sinus[x] = (0x10000000 + (int)move) & 0xFFFFFF;
		int as = (sinus[x] >> 0) & 255;
		B(as)
	}


	int carpos[256];
	LPC("carpos_l")
	for (i = 0; i < 256; i++)
	{
		carpos[i] = 197+pow(1.0f - (float)i / 255.f, 2.0f) * 150;
		B(LB(carpos[i]));
	}
	LPC("carpos_h")
	for (i = 0; i < 256; i++)
	{
		B(HB(carpos[i]));
	}


#if 0
	LPC("screenmodtab1")
	for (x = 0; x < 256; x++)
	{
		int as = (x * 256) % 40;
		B(as);
	}
	LPC("screenmodtab2")
	for (x = 0; x < 256; x++)
	{
		int as = x % 40;
		B(as);
	}
#endif

	PRINTPOS("twister tables");

	value_t twistor = 150 * 40 + 1;
	int twistersize = 19;

	LPC("twistertab_l")
	int twistertab1[256];
	for (x = 0; x < 256; x++)
	{
		int tp = (float)twistersize * (float)x/256.0;
		twistertab1[x] = twistor + tp;
		B(LB(twistertab1[x]));
	}

	LPC("twistertab_h")
	for (x = 0; x < 256; x++)
	{
		B(HB(twistertab1[x]));
	}

	PRINTPOS("column tables");
	value_t values[256];
	LPC("columntab_lb")
	for (i = 0; i < 256; i++)
	{
		x = i & (COLUMNS_PER_BUNDLE - 1);
		int b_in_p = x % 5;
		int page = x / 5;
		values[i] = page * 256 + b_in_p * (25+25+1);
		B(LB(values[i]));
	}
	LPC("columntab_hb")
	for (i = 0; i < 256; i++)
	{
		B(HB(values[i]));
	}

	LPC("bundletab_lb")
	for (i = 0; i < bundle_addr_count; i++)
	{
		B(LB(bundle_addrs[i]));
	}
	LPC("bundletab_hb")
	for (i = 0; i < bundle_addr_count; i++)
	{
		B(HB(bundle_addrs[i]));
	}

	{
		static int randomtab[40];
		int x,y, i;

		if (g_pass == 0)
		{
			for (x = 0; x < 40; x++)
			{
				randomtab[x] = -1;
			}
			int freetab[40];
			int freecount = 0;
			x = 0;
			while(1)
			{
				freecount = 0;
				for (i = 0; i < 40; i++)
				{
					if (randomtab[i] == -1)
					{
						freetab[freecount] = i;
						freecount++;
					}
				}


				int nextfree;
				if (freecount == 1)
				{
					randomtab[x] = 0;
					break;
				}
				else
				{
					int nextfree = freetab[rand() % freecount];
					randomtab[x] = nextfree;
					x = nextfree;
				}

			}

		}

		#if 0
		for (x = 0; x < 40; x++)
		{
			randomtab[x] = (x + 1) % 40;
		}
		#endif

		int cur = randomtab[0];
		printf("cur %i: %i\n", 0, cur);
		for (x = 1; x < 40; x++)
		{
			cur = randomtab[cur];
			printf("cur %i: %i\n", x, cur);
		}

		if (cur != 0)
			exit(1);

		LPC("randomtab")
		for (x = 0; x < 40; x++)
		{
			// printf("%i ", values[x]);
			B(randomtab[x]);
		}
	}

	LPC("spritextab")
	B(0); B(0xFF);

}

void genis()
{
	actualprog();

}


int main()
{
	assemble(genis);

	list<segment_c *> segs2;
	segs2.push_back(&actualprg);
	make_prg("actual.prg", actual_start, segs2);

}
