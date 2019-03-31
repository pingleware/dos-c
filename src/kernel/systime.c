
/****************************************************************/
/*								*/
/*			     systime.c				*/
/*								*/
/*		      DOS/NT Date/Time Functions		*/
/*								*/
/*		        Copyright (c) 1995			*/
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
#include "../../hdr/time.h"
#include "../../hdr/date.h"
#include "globals.h"

/* $Logfile:   D:/dos-c/src/kernel/systime.c_v  $ */
#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Header:   D:/dos-c/src/kernel/systime.c_v   1.3   29 May 1996 21:03:40   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/kernel/systime.c_v  $
 * 
 *    Rev 1.3   29 May 1996 21:03:40   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.2   19 Feb 1996  3:21:34   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.1   01 Sep 1995 17:54:16   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995  8:32:20   patv
 * Initial revision.
 */

static BYTE *clk_name = "CLOCK$";

static WORD days[2][13] =
{
	{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
	{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

static WORD ndays[2][13] =
{
	/*  1   2   3   4   5   6   7   8   9   10  11  12 */
	{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	{0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};

static COUNT
	sysHour = 0,
	sysMinute = 0,
	sysSecond = 0,
	Hundreths = 0;

static COUNT
	Month = EPOCH_MONTH,
	Day = EPOCH_DAY,
	Year = EPOCH_YEAR,
	DayOfWeek = EPOCH_WEEKDAY;

VOID DosGetTime(hp, mp, sp, hdp)
BYTE FAR *hp, FAR *mp, FAR *sp, FAR *hdp;
{
	struct ClockRecord buf;
	request rq;

	rq.r_length = sizeof(request);
	rq.r_command = C_INPUT;
	rq.r_count = sizeof(buf);
	rq.r_trans = (BYTE FAR *)(&buf);
	rq.r_status = 0;
	execrh((request FAR *)&rq, (struct dhdr FAR *)clock);
	if(rq.r_status & S_ERROR)
		return;

	*hp = sysHour = buf.clkHours;
	*mp = sysMinute = buf.clkMinutes;
	*sp = sysSecond = buf.clkSeconds;
	*hdp = Hundreths = buf.clkHundredths;
}


COUNT DosSetTime(hp, mp, sp, hdp)
BYTE FAR *hp, FAR *mp, FAR *sp, FAR *hdp;
{
	struct ClockRecord buf;
	request rq;

	DosGetDate((BYTE FAR *)&DayOfWeek, (BYTE FAR *)&Month,
	 (BYTE FAR *)&Day, (BYTE FAR *)&Year);

	buf.clkHours = sysHour = *hp;
	buf.clkMinutes = sysMinute = *mp;
	buf.clkSeconds = sysSecond = *sp;
	buf.clkHundredths = Hundreths = *hdp;

	buf.clkDays = Day
	 + days[!(Year & 0x3)][Month - 1]
	 + ((Year - 1980) * 365)
	 + ((Year - 1980 + 3) / 4);

	rq.r_length = sizeof(request);
	rq.r_command = C_OUTPUT;
	rq.r_count = sizeof(buf);
	rq.r_trans = (BYTE FAR *)(&buf);
	rq.r_status = 0;
	execrh((request FAR *)&rq, (struct dhdr FAR *)clock);
	if(rq.r_status & S_ERROR)
		return char_error(&rq, clk_name);
	return SUCCESS;
}


VOID DosGetDate(wdp, mp, mdp, yp)
BYTE FAR *wdp, FAR *mp, FAR *mdp;
COUNT FAR *yp;
{
	COUNT count, c;
	struct ClockRecord buf;
	request rq;

	rq.r_length = sizeof(request);
	rq.r_command = C_INPUT;
	rq.r_count = sizeof(buf);
	rq.r_trans = (BYTE FAR *)(&buf);
	rq.r_status = 0;
	execrh((request FAR *)&rq, (struct dhdr FAR *)clock);
	if(rq.r_status & S_ERROR)
		return;

	for(Year = 1980, c = buf.clkDays; c > 0; )
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

	/* c contains the days left and count the number of days for	*/
	/* that year.  Use this to index the table.			*/
	for(Month = 1; Month < 13; Month++)
	{
		if(days[count == 366][Month] > c)
		{
			Day = c - days[count == 366][Month - 1];
			break;
		}
	}

	*mp = Month;
	*mdp = Day;
	*yp = Year;

	/* Day of week is simple. Take mod 7, add 2 (for Tuesday	*/
	/* 1-1-80) and take mod again					*/
	
	DayOfWeek = (buf.clkDays - 1) % 7;
	DayOfWeek += 2;
	DayOfWeek %= 7;
	*wdp = DayOfWeek;
}


COUNT DosSetDate(mp, mdp, yp)
BYTE FAR *mp, FAR *mdp;
COUNT FAR *yp;
{
	struct ClockRecord buf;
	request rq;

	Month = *mp;
	Day = *mdp;
	Year = *yp;
	if(Year < 1980 || Year > 2099
	 || Month < 1 || Month > 12
	 || Day < 1 || Day > ndays[!(Year & 0x3)][Month])
		return DE_INVLDDATA;

	DosGetTime((BYTE FAR *)&sysHour, (BYTE FAR *)&sysMinute,
	 (BYTE FAR *)&sysSecond, (BYTE FAR *)&Hundreths);
	buf.clkHours = sysHour;
	buf.clkMinutes = sysMinute;
	buf.clkSeconds = sysSecond;
	buf.clkHundredths = Hundreths;

	buf.clkDays = Day
	 + days[!(Year & 0x3)][Month - 1]
	 + ((Year - 1980) * 365)
	 + ((Year - 1980 + 3) / 4);

	rq.r_length = sizeof(request);
	rq.r_command = C_OUTPUT;
	rq.r_count = sizeof(buf);
	rq.r_trans = (BYTE FAR *)(&buf);
	rq.r_status = 0;
	execrh((request FAR *)&rq, (struct dhdr FAR *)clock);
	if(rq.r_status & S_ERROR)
		return char_error(&rq, clk_name);
	return SUCCESS;
}

