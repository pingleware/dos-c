
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

/* $Logfile:   D:/dos-c/src/kernel/strings.c_v  $ */
#ifdef VERSION_STRINGS
static BYTE *stringsRcsId = "$Header:   D:/dos-c/src/kernel/strings.c_v   1.4   29 May 1996 21:03:30   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/kernel/strings.c_v  $
 * 
 *    Rev 1.4   29 May 1996 21:03:30   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.3   19 Feb 1996  3:21:36   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.2   01 Sep 1995 17:54:22   patv
 * First GPL release.
 * 
 *    Rev 1.1   30 Jul 1995 20:51:58   patv
 * Eliminated version strings in ipl
 * 
 *    Rev 1.0   02 Jul 1995  8:33:46   patv
 * Initial revision.
 */

#ifdef PROTO
VOID strcpy(REG BYTE *, REG BYTE *);
#endif


COUNT 
strlen (REG BYTE *s)
{
	REG WORD cnt = 0;

	while(*s++ != 0)
		++cnt;
	return cnt;
}


COUNT 
fstrlen (REG BYTE FAR *s)
{
	REG WORD cnt = 0;

	while(*s++ != 0)
		++cnt;
	return cnt;
}


VOID 
strcpy (REG BYTE *d, REG BYTE *s)
{
	while(*s != 0)
		*d++ = *s++;
	*d = 0;
}

VOID 
strncpy (REG BYTE *d, REG BYTE *s, COUNT l)
{
	COUNT idx = 1;
	while(*s != 0 && idx++ <= l)
		*d++ = *s++;
	*d = 0;
}


VOID 
strcat (REG BYTE *d, REG BYTE *s)
{
	while(*d != 0)
		++d;
	strcpy(d, s);
}


COUNT 
strcmp (REG BYTE *d, REG BYTE *s)
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

COUNT 
fstrcmp (REG BYTE FAR *d, REG BYTE FAR *s)
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

COUNT 
strncmp (REG BYTE *d, REG BYTE *s, COUNT l)
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


COUNT 
fstrncmp (REG BYTE FAR *d, REG BYTE FAR *s, COUNT l)
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


VOID
fstrncpy(REG BYTE FAR *d, REG BYTE FAR *s, COUNT l)
{
	COUNT idx = 1;
	while(*s != 0 && idx++ <= l)
		*d++ = *s++;
	*d = 0;
}


/* Yet another change for true portability (WDL)			*/
COUNT 
tolower (COUNT c)
{
	if(c >= 'A' && c <= 'Z')
		return (c + ('a' - 'A'));
	else
		return c;
}


/* Yet another change for true portability (PJV)			*/
COUNT 
toupper (COUNT c)
{
	if(c >= 'a' && c <= 'z')
		return (c - ('a' - 'A'));
	else
		return c;
}





