#include <debugf.h>
#include <hashembler.h>
#include "utils.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"
#include "krill.h"


using namespace hashembler;

segment_asm_c initprg;



const char *whitespace = "\t \r\n";

struct op_t
{
	int type;

	int val;

	int x, y, color;
	char *str;

	int wait;
};

void load_cumbucket(const char *fn)
{
	FILE *f = fopen(fn, "rb");
	if (!f)
	{
		printf("error loading %s", fn);
		return;
	}

	while(!feof(f))
	{
		char rivi[1024];
		char fullrow[1024];

		fgets(rivi, sizeof(rivi), f);
		strcpy(fullrow, rivi);
		if (rivi[0] == '!')
			break;

		op_t op;
		op.type = 0;
		op.val = 0;
		op.wait = 0;
		op.x = 0;
		op.y = 0;
		op.color = 0;
		op.str = NULL;

		char *ptr = rivi;
		int state = 0;
		while(state < 10000)
		{
			size_t siz;
			// skip white space
			siz = strspn(ptr, whitespace);
			ptr += siz;

			// search data length
			if (*ptr == 0 || *ptr == '#')
				break;

			char data[1024];

			if (*ptr != '\"')
			{
				siz = strcspn(ptr, whitespace);
				strncpy(data, ptr, siz);
				data[siz] = 0;
				ptr += siz;
			}
			else
			{
				const char *quotes="\"";

				ptr++;
				siz = strcspn(ptr, quotes);
				strncpy(data, ptr, siz);
				data[siz] = 0;
				ptr += siz + 1;
			}

			switch(state)
			{
			case 0:
				if (!strcmp(data, "nop"))
				{
					op.type = 0;
					state = 500;
				}
				if (!strcmp(data, "setpos"))
				{
					op.type = 1;
					state = 1;
				}
				else if (!strcmp(data, "setvel"))
				{
					op.type = 2;
					state = 1;
				}
				else if (!strcmp(data, "setacc"))
				{
					op.type = 3;
					state = 1;
				}
				else if (!strcmp(data, "wait"))
				{
					op.type = 4;
					state = 1;
				}
				else if (!strcmp(data, "text"))
				{
					op.type = 5;
					state = 10;
				}
				else if (!strcmp(data, "music"))
				{
					op.type = 6;
					state = 1;
				}
				else if (!strcmp(data, "setscreen"))
				{
					op.type = 1;
					state = 20;
				}
				else if (!strcmp(data, "setvolume"))
				{
					op.type = 7;
					state = 1;
				}
				else if (!strcmp(data, "setborder"))
				{
					op.type = 8;
					state = 1;
				}
				else if (!strcmp(data, "setbg"))
				{
					op.type = 9;
					state = 1;
				}
				else if (!strcmp(data, "skip"))
				{
					op.type = 10;
					state = 31;
				}
				else if (!strcmp(data, "label"))
				{
					op.type = 11;
					state = 30;
				}
				else if (!strcmp(data, "textclear"))
				{
					op.type = 12;
					state = 500;
				}
				else if (!strcmp(data, "textcolor"))
				{
					op.type = 13;
					state = 1;
				}
				else if (!strcmp(data, "scrollmode"))
				{
					op.type = 14;
					state = 1;
				}
				else if (!strcmp(data, "settwistage"))
				{
					op.type = 15;
					state = 1;
				}
				else
				{
					state = 10000;
					break;
				}
				break;
			case 1:
				op.val = (int)strtol(data, NULL, 0);
				state = 500;
				break;

			case 10:
				op.x = (int)strtol(data, NULL, 0);
				state = 11;
				break;
			case 11:
				op.y = (int)strtol(data, NULL, 0);
				state = 12;
				break;
			case 12:
				op.color = (int)strtol(data, NULL, 0);
				state = 13;
				break;
			case 13:
				op.str = strdup(data);
				state = 500;
				break;

			case 20:
				op.val = (int)strtol(data, NULL, 0) * 40 * 0x100;
				state = 500;
				break;

			case 30:
				op.str = strdup(data);
				state = 1000;
				break;

			case 31:
				op.val = (int)strtol(data, NULL, 0);
				state = 30;
				break;


			case 500:
				op.wait = (int)strtol(data, NULL, 0);
				state = 1000;
				break;

			case 1000:
				break;
			}
		}

		if (state > 0 && state != 10000)
		{
			fprintf(stderr, "op: %i %04x - '%s' - wait: %i\n", op.type, op.val, op.str, op.wait);

			int b = op.type;
			if (op.wait > 0)
				b |= 0x80;

			if (op.type != 11) // type 11 is label
			{
				B(b);
				if (op.wait > 0)
					B(op.wait);
			}

			switch(op.type)
			{
				case 0:
				case 12:
				break;

				case 1:
				case 2:
				op.val = (0x1000000 + op.val) & 0xFFFFFF;
				B((op.val >> 0) & 0xFF);
				B((op.val >> 8) & 0xFF);
				B((op.val >> 16) & 0xFF);
				break;

				case 3:
				case 4:
				case 15:
				op.val = (0x10000 + op.val) & 0xFFFF;
				B((op.val >> 0) & 0xFF);
				B((op.val >> 8) & 0xFF);
				break;

				case 5:
/*
	// text screen lb
	// text screen hb
	// text color
	// text offset
*/
				{
					value_t mempos = op.y * 40;
					B(LB(mempos));
					B(HB(mempos));
					B(op.color);
					B(op.x);
					SEG->add_string(op.str, &encoder_scr);

					break;
				}

				case 6:
				case 7:
				case 8:
				case 9:
				case 13:
				case 14:
				op.val = (0x100 + op.val) & 0xFF;
				B((op.val >> 0) & 0xFF);
				break;

				case 11:
				LPC(op.str);
				fprintf(stderr, "label: '%s'\n", op.str);
//				exit(1);
				break;

				case 10:
				op.val = (0x10000 + op.val) & 0xFFFF;
				B((op.val >> 0) & 0xFF);
				B((op.val >> 8) & 0xFF);
				B(LB(L(op.str)-1));
				B(HB(L(op.str)-1));
				fprintf(stderr, "skip to label: '%s'\n", op.str);

				break;

				break;
			}

			if (op.str)
			{
				free(op.str);
				op.str = NULL;
			}
		}

	}
}



void cum_exec()
{
	LPC("cum_init")
	MOV16i(cumctx_head, L("cumcode")-1);
	MOV16i(cumctx_nextthink, 0);
	RTS();

	LPC("cum_exec")

/*
	0x0f 00
	0x00 00

	0x00 - 0x00     = 0x00 c=1
	0x0f - 0x00     = 0x0f c=1

	0x00 - 0x00     = 0x00 c=1
	0x0f - 0x0f     = 0x00 c=1

	0x00 - 0x01     = 0xff c=0
	0x0f - 0x0f     = 0x00 c=0

	0x00 - 0x01     = 0xff c=0
	0x0f - 0x0f     = 0x00 c=0



	0x00 - 0x00     = 0x00 c=1
	0x00 - 0x0f     = 0xf1 c=0

	0x00 - 0x00     = 0x00 c=1
	0x0f - 0x0f     = 0x00 c=1

	0x01 - 0x00     = 0x01 c=1
	0x0f - 0x0f     = 0x00 c=1


*/
	LDYi(0);
	LPC("cum_exec_loop")
	LDAz(framecounter + 0)
	CMPz(cumctx_nextthink + 0);
	LDAz(framecounter + 1)
	SBCz(cumctx_nextthink + 1);
	BCS(L("cum_exec_single"));


	LPC("stop_execution");
	TYA();
	BNE(L("fix_head"))
	RTS();

	LPC("fix_head")

	CLC();
	ADCz(cumctx_head + 0)
	STAz(cumctx_head + 0)
	LDAi(0)
	ADCz(cumctx_head + 1)
	STAz(cumctx_head + 1)


	RTS();

	LPC("cum_exec_single")
	INY();
	LDAizy(cumctx_head);
	PRINTPOS("read_inst")
	TAX();
	BPL(L("read_datas"));
	ANDi(0x7F);
	TAX();

	LDAz(framecounter + 0);
	INY();
	CLC();
	ADCizy(cumctx_head);
	STAz(cumctx_nextthink + 0);
	LDAz(framecounter + 1);
	ADCi(0);
	STAz(cumctx_nextthink + 1);

	LPC("read_datas");
	PRINTPOS("read_datas");

	LDAx(L("cumfuncs_h"));
	PHA();
	LDAx(L("cumfuncs_l"));
	PHA();
	RTS();

	LPC("cum_nop");
	PRINTPOS("cum_nop")
	JMP(L("cum_exec_loop"))

	LPC("cum_setpos");
	PRINTPOS("cum_setpos")
	INY();
	LDAizy(cumctx_head);
	STAz(scrollpos + 0);
	INY();
	LDAizy(cumctx_head);
	STAz(scrollpos + 1);
	INY();
	LDAizy(cumctx_head);
	STAz(scrollpos + 2);
	JMP(L("cum_exec_loop"))

	LPC("cum_setval");
	INY();
	LDAizy(cumctx_head);
	STAz(scrollpos_v + 0);
	INY();
	LDAizy(cumctx_head);
	STAz(scrollpos_v + 1);
	INY();
	LDAizy(cumctx_head);
	STAz(scrollpos_v + 2);
	JMP(L("cum_exec_loop"))

	LPC("cum_setacc");
	INY();
	LDAizy(cumctx_head);
	STAz(scrollpos_a + 0);
	INY();
	LDAizy(cumctx_head);
	STAz(scrollpos_a + 1);

	BPL(L("acc_pos"))
	LDAi(0xFF);
	STAz(scrollpos_a + 2);
	JMP(L("cum_exec_loop"))
	LPC("acc_pos");
	LDAi(0);
	STAz(scrollpos_a + 2);
	JMP(L("cum_exec_loop"))

	LPC("cum_wait");
	INY();
	LDAizy(cumctx_head);
	STAz(cumctx_nextthink + 0);
	INY();
	LDAizy(cumctx_head);
	STAz(cumctx_nextthink + 1);
	JMP(L("cum_exec_loop"))

	LPC("cum_text");
	INY();
	// text screen lb
	LDAizy(cumctx_head);
	STAz(text_pos_screen + 0);

	INY();
	// text screen hb
	LDAizy(cumctx_head);
	STAz(text_pos_screen + 1);

	INY();
	// text color
	LDAizy(cumctx_head);
	STAz(text_color)

//	B(2);

	INY();
	// text offset
	LDAizy(cumctx_head);
	TAX();


	LPC("text_read_loop")
	INY();
	LDAizy(cumctx_head);
	BEQ(L("text_read_loop_end"))
	STAx(text_colbuf)
	LDAi(0xFF)
	STAx(colbuf_cur_h)
	INX();
	JMP(L("text_read_loop"));


	LPC("text_read_loop_end");
	JMP(L("cum_exec_loop"))




	LPC("cum_textclear");
	LDAi(0);
	LDXi(39);
	LPC("text_clear_loop")
	STAx(text_colbuf)
	DEX();
	BPL(L("text_clear_loop"));
	LDXi(39);
	LPC("text_clear_loop2")
	STAx(colbuf_cur_h)
	DEX();
	BPL(L("text_clear_loop2"));
	JMP(L("cum_exec_loop"))

	LPC("cum_textcolor");
	INY();
	// text color
	LDAizy(cumctx_head);
	STAz(text_color)

	LDXi(39);
	LPC("text_colorchg_loop")
//	LDAx(text_colbuf)
	DECx(colbuf_cur_h)
	DEX();
	BPL(L("text_colorchg_loop"));
	JMP(L("cum_exec_loop"))





	LPC("cum_music");
	INY();
	TYA();
	PHA();
	LDAizy(cumctx_head);
	JSR(music_init);
	PLA();
	TAY();
	JMP(L("cum_exec_loop"));

	LPC("cum_volume");
	INY();
	TYA();
	PHA();
	LDAizy(cumctx_head);
	JSR(music_vol);
	PLA();
	TAY();
	JMP(L("cum_exec_loop"));

	LPC("cum_border");
	INY();
	LDAizy(cumctx_head);
	STA(0xd020);
	JMP(L("cum_exec_loop"));

	LPC("cum_bg");
	INY();
	LDAizy(cumctx_head);
	STA(0xd021);
	JMP(L("cum_exec_loop"));

	LPC("cum_scrollmode");
	INY();
	LDAizy(cumctx_head);
	STAz(scrollmode);
	JMP(L("cum_exec_loop"));

	LPC("cum_twistage");
	INY();
	LDAizy(cumctx_head);
	STAz(twister_target + 0);
	INY();
	LDAizy(cumctx_head);
	STAz(twister_target + 1);
	JMP(L("cum_exec_loop"));


	// this will break, it manipulates the cumctx_head directly, resets y and returns from execution
	LPC("cum_skip");
	INY();
	LDAizy(cumctx_head);
	STAz(framecounter + 0);
	STAz(cumctx_nextthink + 0);

	INY();
	LDAizy(cumctx_head);
	STAz(framecounter + 1);
	STAz(cumctx_nextthink + 1);

	INY();
	LDAizy(cumctx_head);
	PHA();

	INY();
	LDAizy(cumctx_head);
	STAz(cumctx_head + 1);
	PLA();
	STAz(cumctx_head + 0);

	LDYi(0);

	RTS();


	value_t cumfuncs[] = {L("cum_nop"), L("cum_setpos"), L("cum_setval"), L("cum_setacc"), L("cum_wait"), L("cum_text"),
						  L("cum_music"), L("cum_volume"), L("cum_border"), L("cum_bg"), L("cum_skip"), 0, L("cum_textclear"),
						  L("cum_textcolor"), L("cum_scrollmode"), L("cum_twistage")};
	int cumfuncnum = sizeof(cumfuncs) / sizeof(cumfuncs[0]);
	printf("%i cumfuncs\n", cumfuncnum);
	int i;

	LPC("cumfuncs_h");
	for (i = 0; i < cumfuncnum; i++)
	{
		B(HB(cumfuncs[i]-1))
	}

	LPC("cumfuncs_l");
	for (i = 0; i < cumfuncnum; i++)
	{
		B(LB(cumfuncs[i]-1))
	}
}

void prg()
{
	CTX("cumvm")
	initprg.begin(0xE000);
	set_segment(&initprg);
	LPC("begin");
	JMP(L("cum_init"))
	JMP(L("cum_exec"))

	SEG->add_string("A MESSAGE TO IAN COOG... LASERBOY WAS OUR PROSPECT MEMBER, "
		"BUT HE WASN'T ACCEPTED TO THE GROUP LINEUP BECAUSE HE SIMPLY WASN'T COOL ENOUGH");

	cum_exec();

	LPC("cumcode")
	load_cumbucket("script.cum");

	int i;
	for (i = 0; i < 1000; i++)
	{
		B(0);
	}

}

void genis()
{
	prg();
}

int main()
{
	assemble(genis);

	list<segment_c *> segs;
	segs.push_back(&initprg);
	make_prg("cumvm.prg", 0xE000, segs);
}
