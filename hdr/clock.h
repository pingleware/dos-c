
/****************************************************************/
/*								*/
/*			      clock.h				*/
/*								*/
/*	     Clock Driver data structures & declarations	*/
/*								*/
/*			November 26, 1991			*/
/*								*/
/*		   Adapted to DOS/NT June 12, 1993		*/
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


#ifdef MAIN
# ifdef VERSION_STRINGS
static BYTE *clock_hRcsId = "$Header:   D:/dos-c/hdr/clock.h_v   1.4   29 May 1996 21:25:20   patv  $";
# endif
#endif

/*
 * $Log:   D:/dos-c/hdr/clock.h_v  $
 *	
 *	   Rev 1.4   29 May 1996 21:25:20   patv
 *	bug fixes for v0.91a
 *	
 *	   Rev 1.3   19 Feb 1996  3:15:30   patv
 *	Added NLS, int2f and config.sys processing
 *	
 *	   Rev 1.2   01 Sep 1995 17:35:38   patv
 *	First GPL release.
 *	
 *	   Rev 1.1   30 Jul 1995 20:43:46   patv
 *	Eliminated version strings in ipl
 *	
 *	   Rev 1.0   02 Jul 1995 10:39:26   patv
 *	Initial revision.
 */

struct ClockRecord
{
	UWORD	clkDays;	/* days since Jan 1, 1980.		*/
	UBYTE	clkMinutes;	/* residual minutes.			*/
	UBYTE	clkHours;	/* residual hours.			*/
	UBYTE	clkHundredths;	/* residual hundredths of a second.	*/
	UBYTE	clkSeconds;	/* residual seconds.			*/
};

