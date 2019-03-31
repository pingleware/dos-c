
/****************************************************************/
/*								*/
/*			     sysclk.c				*/
/*								*/
/*		       System Clock Driver			*/
/*								*/
/*			Copyright (c) 1995			*/
/*			Pasquale J. Villani			*/
/*			All Rights Reserved			*/
/*								*/
/* This file is part of DOS-C.					*/
/*								*/
/* DOS-C is free software; you can redistribute it and/or	*/
/* modify it under the terms of the GNU General Public License	*/
/* as published by the Free Software Foundation; either version	*/
/* 2, or (at your option) any later version.			*/
/*								*/
/* DOS-C is distributed in the hope that it will be useful, but	*/
/* WITHOUT ANY WARRANTY; without even the implied warranty of	*/
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See	*/
/* the GNU General Public License for more details.		*/
/*								*/
/* You should have received a copy of the GNU General Public	*/
/* License along with DOS-C; see the file COPYING.  If not,	*/
/* write to the Free Software Foundation, 675 Mass Ave,		*/
/* Cambridge, MA 02139, USA.					*/
/****************************************************************/

#include "../../hdr/portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Header:   D:/dos-c/src/kernel/sysclk.c_v   1.3   29 May 1996 21:03:48   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/kernel/sysclk.c_v  $
 * 
 *    Rev 1.3   29 May 1996 21:03:48   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.2   19 Feb 1996  3:21:34   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.1   01 Sep 1995 17:54:18   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995  8:32:30   patv
 * Initial revision.
 */

#ifdef PROTO
COUNT ReadATClock(BYTE *, BYTE *, BYTE *, BYTE *);
BOOL ReadPCClock(ULONG *);
VOID WriteATClock(BYTE *, BYTE, BYTE, BYTE);
VOID WritePCClock(ULONG);
COUNT BcdToByte(COUNT);
COUNT BcdToWord(BYTE *, UWORD *, UWORD *, UWORD *);
COUNT ByteToBcd(COUNT);
LONG WordToBcd(BYTE *, UWORD *, UWORD *, UWORD *);
#else
COUNT ReadATClock();
BOOL ReadPCClock();
VOID WriteATClock();
VOID WritePCClock();
COUNT BcdToByte();
COUNT BcdToWord();
COUNT ByteToBcd();
LONG WordToBcd();
#endif

/*									*/
/* WARNING - THIS DRIVER IS NON-PORTABLE!!!!				*/
/*									*/

static WORD days[2][13] =
{
	{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
	{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

static struct ClockRecord clk;

static BYTE bcdDays[4];
static UWORD Month, Day, Year;
static BYTE bcdMinutes;
static BYTE bcdHours;
static BYTE bcdHundredths;
static BYTE bcdSeconds;

static ULONG Ticks;
static UWORD DaysSinceEpoch;

WORD 
clk_driver (rqptr rp)
{
	REG COUNT count, c;
	BYTE FAR *cp;

	switch(rp -> r_command)
	{
	case C_INIT:
		clk.clkDays = DaysSinceEpoch = 1;
		clk.clkMinutes = 0;
		clk.clkHours = 0;
		clk.clkHundredths = 0;
		clk.clkSeconds = 0;
		rp -> r_endaddr = device_end();
		rp -> r_nunits = 0;
		return S_DONE;


	case C_INPUT:
		count = rp -> r_count;
		if(count > sizeof(struct ClockRecord))
			count = sizeof(struct ClockRecord);
		if(!ReadATClock(bcdDays, &bcdHours, &bcdMinutes, &bcdSeconds))
		{
			/* AT - deal with it				*/
			clk.clkDays = BcdToWord(bcdDays, &Month, &Day, &Year);
			clk.clkMinutes = BcdToByte(bcdMinutes);
			clk.clkHours = BcdToByte(bcdHours);
			clk.clkHundredths = BcdToByte(bcdHundredths);
			clk.clkSeconds = BcdToByte(bcdSeconds);
		}
		else
		{
			/* PC - deal with it				*/
			UCOUNT remainder;

			if(!ReadPCClock(&Ticks))
				++DaysSinceEpoch;
			clk.clkDays = DaysSinceEpoch;
			clk.clkHours = Ticks / 65520l;
			remainder = Ticks % 65520l;
			clk.clkMinutes = (remainder) / 1092;
			remainder %= 1092;
			clk.clkSeconds = (remainder * 10) / 182;
			remainder %= 182;
			clk.clkHundredths = (remainder * 100) / 182;
		}
		fbcopy((BYTE FAR *)&clk, rp -> r_trans, count);
		return S_DONE;

	case C_OUTPUT:
		count = rp -> r_count;
		if(count > sizeof(struct ClockRecord))
			count = sizeof(struct ClockRecord);
		rp -> r_count = count;
		fbcopy(rp -> r_trans, (BYTE FAR *)&clk, count);

		/* Set PC Clock first					*/
		DaysSinceEpoch = clk.clkDays;
		Ticks = (LONG)clk.clkHours * 65520l
		 + (LONG)clk.clkMinutes * 1092l
		 + (LONG)clk.clkSeconds * 18l;
		WritePCClock(Ticks);

		/* Now set AT clock					*/
		/* Fix year by looping through each year, subtracting	*/
		/* the appropriate number of days for that year.	*/
		for(Year = 1980, c = clk.clkDays; c > 0; )
		{
			 count = !(Year & 0x3) ? 366 : 365;
			 if(c > count)
			 {
				++Year;
				c -= count;
			 }
			 else
				break;
		}

		/* c contains the days left and count the number of	*/
		/* days for that year.  Use this to index the table.	*/
		for(Month = 1; Month < 13; ++Month)
		{
			if(days[count == 366][Month] > c)
			{
				Day = c - days[count == 366][Month - 1];
				break;
			}
		}
		WordToBcd((BYTE *)bcdDays, &Month, &Day, &Year);
		bcdMinutes = ByteToBcd(clk.clkMinutes);
		bcdHours = ByteToBcd(clk.clkHours);
		bcdSeconds = ByteToBcd(clk.clkSeconds);
		WriteATClock(bcdDays, bcdHours, bcdMinutes, bcdSeconds);
		return S_DONE;

	case C_OFLUSH:
	case C_IFLUSH:
		return S_DONE;

	case C_OUB:
	case C_NDREAD:
	case C_OSTAT:
	case C_ISTAT:
	default:
		return failure(E_FAILURE);	/* general failure */
	}
}


COUNT 
BcdToByte (COUNT x)
{
	return ((((x)>>4)&0xf)*10+((x)&0xf));
}


COUNT 
BcdToWord (BYTE *x, UWORD *mon, UWORD *day, UWORD *yr)
{
	*mon = BcdToByte(x[1]);
	*day = BcdToByte(x[0]);
	*yr = BcdToByte(x[3]) * 100 + BcdToByte(x[2]);
	if(*yr < 1980)
		return -1;
	else
		return *day + days[!(*yr & 0x3)][*mon - 1] + ((*yr - 1980) * 365) + ((*yr - 1980 + 3) / 4);
}


COUNT 
ByteToBcd (COUNT x)
{
	return ((x / 10) << 4) | (x % 10);
}


LONG 
WordToBcd (BYTE *x, UWORD *mon, UWORD *day, UWORD *yr)
{
	x[1] = ByteToBcd(*mon);
	x[0] = ByteToBcd(*day);
	x[3] = ByteToBcd(*yr / 100);
	x[2] = ByteToBcd(*yr % 100);
}


