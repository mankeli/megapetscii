#pragma once
#include <hashembler.h>

namespace hashembler
{

static void MOV8i(value_t outaddr, value_t inval)
{
	LDAi(inval);
	STA(outaddr);
}

static void MOV16(value_t outaddr, value_t inaddr)
{
	LDA(inaddr + 0);
	STA(outaddr + 0);
	LDA(inaddr + 1);
	STA(outaddr + 1);
}

static void MOV16z(value_t outaddr, value_t inaddr)
{
	LDAz(inaddr + 0);
	STAz(outaddr + 0);
	LDAz(inaddr + 1);
	STAz(outaddr + 1);
}
static void MOV16i(value_t outaddr, value_t inval)
{
	LDAi(LB(inval));
	STA(outaddr + 0);
	LDAi(HB(inval));
	STA(outaddr + 1);
}

static void ADD16z_i(value_t addr, value_t con)
{
	LDAz(addr + 0);
	CLC();
	ADCi(LB(con));
	STAz(addr + 0);
	LDAz(addr + 1);
	ADCi(HB(con));
	STAz(addr + 1);
}


}