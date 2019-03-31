
/****************************************************************/
/*								*/
/*			     misc.c				*/
/*								*/
/*		   Miscellaneous Kernel Functions		*/
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

#include "../../hdr/portab.h"

/* $Logfile:   D:/dos-c/src/fs/misc.c_v  $ */
#ifndef IPL
static BYTE *miscRcsId = "$Header:   D:/dos-c/src/fs/misc.c_v   1.4   29 May 1996 21:15:18   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/fs/misc.c_v  $
 * 
 *    Rev 1.4   29 May 1996 21:15:18   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.3   19 Feb 1996  3:20:12   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.2   01 Sep 1995 17:48:46   patv
 * First GPL release.
 * 
 *    Rev 1.1   30 Jul 1995 20:50:28   patv
 * Eliminated version strings in ipl
 * 
 *    Rev 1.0   02 Jul 1995  8:06:28   patv
 * Initial revision.
 */

#include "globals.h"


VOID 
scopy (REG BYTE *s, REG BYTE *d)
{
	while(*s)
		*d++ = *s++;
	*d = '\0';
}


VOID
fscopy(REG BYTE FAR *s, REG BYTE FAR *d)
{
	while(*s)
		*d++ = *s++;
	*d = '\0';
}


VOID
fsncopy(BYTE FAR *s, BYTE FAR *d, REG COUNT n)
{
	while(*s && n--)
		*d++ = *s++;
	*d = '\0';
}


#ifndef ASMSUPT
VOID 
bcopy (REG BYTE *s, REG BYTE *d, REG COUNT n)
{
	while(n--)
		*d++ = *s++;
}


VOID
fbcopy(REG VOID FAR *s, REG VOID FAR *d, REG COUNT n)
{
	while(n--)
		*((BYTE FAR *)d)++ = *((BYTE FAR *)s)++;
}
#endif


BYTE *
skipwh (BYTE *s)
{
	while(*s && (*s == 0x0d || *s == 0x0a || *s == ' ' || *s == '\t'))
		++s;
	return s;
}


BYTE *
scan (BYTE *s, BYTE *d)
{
	s = skipwh(s);
	while(*s &&
	 !( *s == 0x0d
	 || *s == 0x0a
	 || *s == ' '
	 || *s == '\t'
	 || *s == '='))
		*d++ = *s++;
	*d = '\0';
	return s;
}


BOOL
isnum(BYTE *pLine)
{
	return (*pLine >= '0' && *pLine <= '9');
}


BYTE *
GetNumber(REG BYTE *pszString, REG COUNT *pnNum)
{
	*pnNum = 0;
	while(isnum(pszString))
		*pnNum = *pnNum * 10 + (*pszString++ - '0');
	return pszString;
}


BYTE *
scan_seperator (BYTE *s, BYTE *d)
{
	s = skipwh(s);
	if(*s)
		*d++ = *s++;
	*d = '\0';
	return s;
}


