/****************************************************************/
/*								*/
/*			     time.c				*/
/*								*/
/*		        DOS "time" Command 			*/
/*								*/
/*			 December 16, 1991			*/
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

/* $Logfile:   C:/dos-c/src/command/time.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/time.c_v  $ 
 * 
 *    Rev 1.2   29 Aug 1996 13:06:58   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:44   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:02:24   patv
 * Initial revision.
 */

static char *RcsId = "$Header:   C:/dos-c/src/command/time.c_v   1.2   29 Aug 1996 13:06:58   patv  $";

#include "../../hdr/portab.h"
#include "../../hdr/error.h"
#include "globals.h"
#include "proto.h"

extern BYTE *tail;

#ifdef PROTO
BOOL parse_time(BYTE *);
#else
BOOL parse_time();
#endif


BOOL cmd_time(VOID)
{
	BYTE am_pm, c;
	BYTE time_in[MAX_CMDLINE] = "";
	BOOL parse_time();
	VOID set_time();
	COUNT hour;
	COUNT min;
	COUNT sec;
	COUNT hdths;
	BYTE *p, *q;

	for(p = tail, q = time_in; *p && *p != '\r' && *p != '\n'; )
		*q++ = *p++;
	*q = '\0';

	DosGetTime(&hour, &min, &sec, &hdths);

	if(hour >=12)
	{
		hour -= 12;
		am_pm = 'p';
	}
	else
		am_pm = 'a';

	/* user setting time check it */
	if(time_in[0] != '\0')
	{
		while(!parse_time(time_in))
		{
			error_message(INV_TIME);
			printf("Enter new time: ");
			DosRead(STDIN, (BYTE FAR *)time_in, MAX_CMDLINE);
			if(*time_in == '\r')
				break;
		}
	}
	else
	{
		printf("Current time is %-2d:%02d:%02d.%02d%c\nEnter new time: ", hour == 0 ? 12 : hour, min, sec, hdths, am_pm);
		DosRead(STDIN, (BYTE FAR *)time_in, MAX_CMDLINE);
		if(*time_in == '\r')
		{
			printf("\n");
			return TRUE;
		}
		while(!parse_time(time_in))
		{
			error_message(INV_TIME);
			printf("Enter new time: ");
			DosRead(STDIN, (BYTE FAR *)time_in, MAX_CMDLINE);
			if(*time_in == '\r')
				break;
		}
	}
	printf("\n");
	return TRUE;
}

BOOL parse_time(s)
BYTE *s;
{
	COUNT hr = 0, min = 0, sec = 0, hndth = 0;
	BYTE am_pm = ' ';

	if(isnum(*s))
	{
		while(isnum(*s))
		{
			hr *= 10;
			hr += tonum(*s++);
		}
	}
	else
		return FALSE;
	if(*s == ':')
		++s;
	else
		return FALSE;
	if(isnum(*s))
	{
		while(isnum(*s))
		{
			min *= 10;
			min += tonum(*s++);
		}
		if(*s == ':')
		{
			++s;

			if(isnum(*s))
			{
				while(isnum(*s))
				{
					sec *= 10;
					sec += tonum(*s++);
				}
			}
			if(*s == '.')
			{
				++s;

				if(isnum(*s))
				{
					while(isnum(*s))
					{
						hndth *= 10;
						hndth += tonum(*s++);
					}
				}
			}
		}
	}
	if(*s == 'p' || *s == 'P' || *s == 'a' || *s == 'A')
	{
		am_pm = *s++;
	}
	if(*s != '\r' && !(*s == 'm' || *s == 'M'))
		return FALSE;

	if((am_pm == ' ' && (hr < 0 || hr > 24)) ||
		(am_pm == 'a' && hr > 12)
		|| (am_pm == 'p' && !(hr >= 1 && hr <= 12)))
			return FALSE;

	if(min < 0 || min > 60 || sec < 0 || sec > 60 || hndth < 0 || hndth > 99 )
		return FALSE;

	if(am_pm == 'p' || am_pm == 'P')
		hr += 12;
	if((am_pm == 'a' || am_pm == 'A') && hr == 12)
		hr = 0;
	return DosSetTime(&hr, &min, &sec, &hndth) == SUCCESS;
}
