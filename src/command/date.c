/****************************************************************/
/*								*/
/*			     date.c				*/
/*								*/
/*		     command.com "date" Command			*/
/*								*/
/*			 November 6, 1991			*/
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


/* $Logfile:   C:/dos-c/src/command/date.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/date.c_v  $
 * 
 *    Rev 1.2   29 Aug 1996 13:06:52   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:36   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:01:50   patv
 * Initial revision.
 */

#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

static BYTE *RcsId = "$Header:   C:/dos-c/src/command/date.c_v   1.2   29 Aug 1996 13:06:52   patv  $";

static BYTE *day_of_wk[7] =
	{ "Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat" };

static COUNT day_per_mm[2][13] =
	{
		{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 30 },
		{0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 30 }
	};



#ifdef PROTO
BOOL parse_date(BYTE *);
#else
BOOL parse_date();
#endif


BOOL cmd_date(argc, argv)
COUNT argc;
BYTE **argv;
{
	BYTE date_str[MAX_CMDLINE] = "";
	BOOL DosSetDate();
	BOOL parse_date();
	COUNT month, day, year, weekday;

	if(argc == 1)
	{
		BOOL ret;

		/* display the date and get new date */
		DosGetDate(&weekday, &month, &day, &year);

		printf("Current date is %s %d-%d-%d\nEnter date(mm-dd-yy): ",
		day_of_wk[weekday], month, day, year);

		ret = parse_date(date_str);
		printf("\n");
		return ret;
	}

	if(argc == 2)
	{
		printf("\n");
		return parse_date(argv[1]);
	}
	else
	{
		error_message(INV_NUM_PARAMS);
		return FALSE;
	}
}

BOOL parse_date(s)
BYTE *s;
{
	COUNT Month, MonthDay, Year, leap;
	Month = MonthDay = Year = 0;

	if(*s == '\0')
	{
		DosRead(STDIN, s, MAX_CMDLINE);
		if(*s == '\0')
			return TRUE;
	}

	if(isnum(*s))
	{
		while(isnum(*s))
		{
			Month *= 10;
			Month += tonum(*s++);
		}
	}
	else
		return FALSE;
	if(*s == '/' || *s == '.' || *s == '-')
		++s;
	else
		return FALSE;
	if(isnum(*s))
	{
		while(isnum(*s))
		{
			MonthDay *= 10;
			MonthDay += tonum(*s++);
		}
		if(*s == '/' || *s == '.' || *s == '-')
		{
			++s;

			if(isnum(*s))
			{
				while(isnum(*s))
				{
					Year *= 10;
					Year += tonum(*s++);
				}
			}
		}
	}


	if(Year >= 0 && Year <= 99)
		Year = 1900 + Year;

	leap = ((Year%4 == 0) && (Year%100 != 0)) || (Year%400 == 0);
	if( !((Month >= 1 && Month <= 12) &&
	    (MonthDay >=1 && MonthDay <= day_per_mm[leap][Month]) &&
	     (Year >= 1980 && Year <= 2099)) )
	{
		error_message(INV_DATE);
		return FALSE;
	}
	/* all ok set the date */
	return DosSetDate(&Month, &MonthDay, &Year) == SUCCESS;
}
