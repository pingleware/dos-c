
/****************************************************************/
/*								*/
/*			    strings.c				*/
/*								*/
/*		  Global String Handling Functions		*/
/*								*/
/*			Copyright (c) 1995			*/
/*		 	Pasquale J. Villani			*/
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

/* $Logfile:   C:/dos-c/src/misc/strings.c_v  $ */
static BYTE *stringsRcsId = "$Header:   C:/dos-c/src/misc/strings.c_v   1.2   29 Aug 1996 13:07:36   patv  $";

/*
 * $Log:   C:/dos-c/src/misc/strings.c_v  $
 *	
 *	   Rev 1.2   29 Aug 1996 13:07:36   patv
 *	Bug fixes for v0.91b
 *	
 *	   Rev 1.1   01 Sep 1995 18:11:02   patv
 *	First GPL release.
 *	
 *	   Rev 1.0   02 Jul 1995 11:04:42   patv
 *	Initial revision.
 */

#ifdef PROTO
VOID strcpy(REG BYTE *, REG BYTE *);
#endif


COUNT strlen(s)
REG BYTE *s;
{
	REG WORD cnt = 0;

	while(*s++ != 0)
		++cnt;
	return cnt;
}


COUNT fstrlen(s)
REG BYTE FAR *s;
{
	REG WORD cnt = 0;

	while(*s++ != 0)
		++cnt;
	return cnt;
}


VOID strcpy(d, s)
REG BYTE *d, *s;
{
	while(*s != 0)
		*d++ = *s++;
	*d = 0;
}

VOID strncpy(d, s, l)
REG BYTE *d, *s;
COUNT l;
{
	COUNT idx = 1;
	while(*s != 0 && idx++ <= l)
		*d++ = *s++;
	*d = 0;
}


VOID strcat(d, s)
REG BYTE *d, *s;
{
	while(*d != 0)
		++d;
	strcpy(d, s);
}


COUNT strcmp(d, s)
REG BYTE *d, *s;
{
	while(*s != '\0' && *d != '\0')
	{
		if(*d == *s)
			++s, ++d;
		else
			return *d - *s;
	}
	return *d - *s;
}

COUNT strncmp(d, s, l)
REG BYTE *d, *s;
COUNT l;
{
	COUNT index = 1;
	while(*s != '\0' && *d != '\0' && index++ <= l)
	{
		if(*d == *s)
			++s, ++d;
		else
			return *d - *s;
	}
	return *d - *s;
}


VOID fstrncpy(d, s, l)
REG BYTE FAR *d, FAR *s;
COUNT l;
{
	COUNT idx = 1;
	while(*s != 0 && idx++ <= l)
		*d++ = *s++;
	*d = 0;
}


/* Yet another change for true portability (WDL)			*/
COUNT tolower(c)
COUNT c;
{
	if(c >= 'A' && c <= 'Z')
		return (c + ('a' - 'A'));
	else
		return c;
}


/* Yet another change for true portability (PJV)			*/
COUNT toupper(c)
COUNT c;
{
	if(c >= 'a' && c <= 'z')
		return (c - ('a' - 'A'));
	else
		return c;
}





