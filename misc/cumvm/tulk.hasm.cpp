#include <debugf.h>
#include <hashembler.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../lib.h"


using namespace hashembler;

segment_basic_c basicstub;
segment_asm_c initprg;

value_t framecounter = 0x02;

value_t cumctx_head = 0x04;
value_t cumctx_nextthink = 0x06;

value_t scrollpos = 0x08;
value_t scrollvel = 0x0B;
value_t scrollacc = 0x0E;


const char *whitespace = "\t \r\n";

struct op_t
{
	int type;

	int val;

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
					state = 1000;
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
				op.val = (int)strtol(data, NULL, 0);
				state = 11;
				break;
			case 11:
				op.val += (int)strtol(data, NULL, 0) * 40;
				state = 12;
				break;
			case 12:
				op.str = strdup(data);
				state = 500;
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

			B(b);
			if (op.wait > 0)
				B(op.wait);

			switch(op.type)
			{
				case 0:
				break;
				case 1:
				case 2:
				case 3:
				B((op.val >> 0) & 0xFF);
				B((op.val >> 8) & 0xFF);
				B((op.val >> 16) & 0xFF);
				break;
				case 4:
				case 5:
				B((op.val >> 0) & 0xFF);
				B((op.val >> 8) & 0xFF);
				break;
			}

			if (op.str)
				free(op.str);
		}

	}
}



void cum_exec()
{


LPC("cum_exec")

LDYi(0);
LPC("cum_exec_loop")
LDAz(framecounter + 0)
CMPz(cumctx_nextthink + 0);
LDAz(framecounter + 1)
SBCz(cumctx_nextthink + 1);
BCS(L("cum_exec_single"));


LPC("stop_execution");
TYAa();
CLCa();
ADCz(cumctx_head + 0)
STAz(cumctx_head + 0)
LDAi(0)
ADCz(cumctx_head + 1)
STAz(cumctx_head + 1)

RTSa();

LPC("cum_exec_single")
LDAizy(cumctx_head);
PCNOW("read_inst")
TAXa();
BPL(L("read_datas"));
ANDi(0x7F);
TAXa();

LDAz(framecounter + 0);
INYa();
CLCa();
ADCizy(cumctx_head);
STAz(cumctx_nextthink + 0);
LDAz(framecounter + 1);
ADCi(0);
STAz(cumctx_nextthink + 1);

LPC("read_datas");
PCNOW("read_datas");

INYa();
LDAx(L("cumfuncs_h"));
PHAa();
LDAx(L("cumfuncs_l"));
PHAa();
RTSa();

LPC("cum_nop");
PCNOW("cum_nop")
JMP(L("cum_exec_loop"))

LPC("cum_setpos");
PCNOW("cum_setpos")
LDAizy(cumctx_head);
INYa();
STAz(scrollpos + 0);
LDAizy(cumctx_head);
INYa();
STAz(scrollpos + 1);
LDAizy(cumctx_head);
INYa();
STAz(scrollpos + 2);
JMP(L("cum_exec_loop"))

LPC("cum_setval");
LDAizy(cumctx_head);
INYa();
STAz(scrollvel + 0);
LDAizy(cumctx_head);
INYa();
STAz(scrollvel + 1);
LDAizy(cumctx_head);
INYa();
STAz(scrollvel + 2);
JMP(L("cum_exec_loop"))

LPC("cum_setacc");
LDAizy(cumctx_head);
INYa();
LDAz(scrollacc + 0);
ADCizy(cumctx_head);
INYa();
STAz(scrollacc + 1);
LDAizy(cumctx_head);
INYa();
STAz(scrollacc + 2);
JMP(L("cum_exec_loop"))

LPC("cum_wait");
LDAizy(cumctx_head);
INYa();
STAz(cumctx_nextthink + 0);
LDAizy(cumctx_head);
INYa();
STAz(cumctx_nextthink + 1);
JMP(L("cum_exec_loop"))

LPC("cum_text");
JMP(L("cum_exec_loop"))


value_t cumfuncs[] = {L("cum_nop"), L("cum_setpos"), L("cum_setval"), L("cum_setacc"), L("cum_wait"), L("cum_text")};
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
	basicstub.begin(0x801);
	basicstub.add_sys(666, SEGLABEL(initprg, "begin"));
	basicstub.add_end();

	initprg.begin(0x1000);
	set_segment(&initprg);
	LPC("begin");

	SEIa();

	MOV16i(framecounter, 0);
	MOV16i(cumctx_head, L("cumcode"));
	MOV16i(cumctx_nextthink, 0);


	LPC("jumi");

	LDA(framecounter + 1)
	JSR(L("drawhex"))
	LDA(framecounter + 0)
	JSR(L("drawhex"))

	LDAi(' ')
	JSR(0xffd2)

	LDAi('H')
	JSR(0xffd2)
	LDAi(' ')
	JSR(0xffd2)
	LDA(cumctx_head + 1)
	JSR(L("drawhex"))
	LDA(cumctx_head + 0)
	JSR(L("drawhex"))

	LDAi(' ')
	JSR(0xffd2)
	LDAi(' ')
	JSR(0xffd2)

	LDAi('N')
	JSR(0xffd2)
	LDAi('T')
	JSR(0xffd2)
	LDAi(' ')
	JSR(0xffd2)

	LDA(cumctx_nextthink + 1)
	JSR(L("drawhex"))
	LDA(cumctx_nextthink + 0)
	JSR(L("drawhex"))

	LDAi('\r')
	JSR(0xffd2)

	JSR(L("cum_exec"))

	LPC("wait_key")
	JSR(0xFFE4)
	BEQ(L("wait_key"))

	ADD16z_i(framecounter, 0x01);

	JMP(L("jumi"))

	LPC("drawhex")
	PHAa();
	LSRa();
	LSRa();
	LSRa();
	LSRa();
	TAXa();
	LDAx(L("hextab"))
	JSR(0xffd2);
	PLAa();
	ANDi(0x0F);
	TAXa();
	LDAx(L("hextab"))
	JSR(0xffd2);
	RTSa();

	LPC("hextab")
	SEG.add_string("0123456789ABCDEF", 16);

	cum_exec();

	LPC("cumcode")
	load_cumbucket("script.cum");

}

void genis()
{
	prg();
}


int main()
{
	assemble(genis);

	list<segment_c *> segs;
	segs.push_back(&basicstub);
	segs.push_back(&initprg);
	make_prg("tulk.prg", 0x0801, segs);
}
