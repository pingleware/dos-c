/****************************************************************/
/*								*/
/*			      num.c				*/
/*								*/
/*		       DOS Numeric Functions			*/
/*								*/
/*			  August 2, 1993			*/
/*								*/
/*			Copyright (c) 1993			*/
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


/* $Logfile:   C:/dos-c/src/misc/num.c_v  $ */
static char *RcsId = "$Header:   C:/dos-c/src/misc/num.c_v   1.2   29 Aug 1996 13:07:38   patv  $";

/*
 * $Log:   C:/dos-c/src/misc/num.c_v  $
 *	
 *	   Rev 1.2   29 Aug 1996 13:07:38   patv
 *	Bug fixes for v0.91b
 *	
 *	   Rev 1.1   01 Sep 1995 18:11:02   patv
 *	First GPL release.
 *	
 *	   Rev 1.0   02 Jul 1995 11:04:48   patv
 *	Initial revision.
 */

#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

BOOL isnum(c)
COUNT c;
{
	/* The following will need to be converted to portable (not	*/
	/* ascii dependant) soon.					*/
	return (c >= '0' && c <= '9');
}


COUNT tonum(c)
COUNT c;
{
	if(isnum(c))
		return (c - '0');
	else
		return 0;
}

COUNT atoi(s)
BYTE s[];
{
	COUNT i, n, sign;

	for(i = 0; isspace(s[i]); i++)
		;
	sign = (s[i] == '-') ? -1 : 1;

	if(s[i] == '+' || s[i] == '-')
		i++;

	for(n = 0; isdigit(s[i]); i++)
		n = 10 * n + (s[i] - '0');

	return sign * n;
}
