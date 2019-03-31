/****************************************************************/
/*								*/
/*			     scan.c				*/
/*								*/
/*		     command.com lexical support		*/
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

/* $Logfile:   C:/dos-c/src/command/scan.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/scan.c_v  $ 
 * 
 *    Rev 1.2   29 Aug 1996 13:06:56   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:42   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:02:18   patv
 * Initial revision.
 */

static char *RcsId = "$Header:   C:/dos-c/src/command/scan.c_v   1.2   29 Aug 1996 13:06:56   patv  $";

#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

VOID scopy(s, d)
REG BYTE *s, *d;
{
	while(*s)
		*d++ = *s++;
	*d = '\0';
}


VOID expand(d, s)
REG BYTE *d, *s;
{
	COUNT idx, bufidx;
	BYTE buffer[MAX_CMDLINE];

	*d = '\0';
	while(*s)
	{
		if(*s == '%' && isnum(s[1]))
		{
			idx = tonum(*++s);
			idx = (idx == 0 ? 0 : idx + shift_offset);
			strcpy(d, posparam[idx]);
			d += strlen(posparam[idx]);
			while(*s && !(*s == 0x0d || *s == 0x0a || *s == ' ' || *s == '\t' || *s == '%'))
				++s;
		}
		else if(*s == '%' && *(s + 1) == '%')
		{
			/* swallow one % save the other			*/
			*d++ = *s++;
			s++;
		}
		else if(*s == '%')
		{
			/* get passed the % 				 */
			++s;
			/* buffer until next %, see if string between the */
			/* is an environment variable			  */
			/* if lp iswhite its not an env var, if its %     */
			/* then look in env and substitute if found	  */
			bufidx = 0;
			while(*s && !(*s == 0x0d || *s == 0x0a || *s == ' ' || *s == '\t' || *s == '%'))
			{
				buffer[bufidx++] = *s++;
				buffer[bufidx] = '\0';
			}
			if(*s != '%')
			{
			/* can't be env variable so add to cmd line	  */
				*d++ = '%';
				strcpy(d, buffer);
				d += strlen(buffer);
			}
			else
			{
			/* lookup and substitiue			  */
			/* get passed ending %				  */
				strcpy(d, EnvLookup(buffer));
				d += strlen(EnvLookup(buffer));
				s++;
			}
		}
		else
		{
			*d++ = *s++;
			*d = '\0';
		}
	}
}

VOID expandspl(d, s, var, sub)
REG BYTE *s, *d;
COUNT var;
BYTE *sub;
{
	while(*s)
	{
		if(*s == '%' && s[1] == var)
		{
			strcpy(d, sub);
			d += strlen(sub);
			while(*s == '%')
				++s;
			while(*s && !(*s == 0x0d || *s == 0x0a || *s == ' ' || *s == '\t' || *s == '%'))
				++s;
		}
		if((*s == '%' && s[1] != var)
		|| (*s == '%' && s[1] == '%' && s[2] != var))
		{
			++s;	/* Throw away leading %			*/
			*d++ = *s++;
			*d = '\0';
		}
		else
		{
			*d++ = *s++;
			*d = '\0';
		}
	}
}

BYTE *scan(s, d)
BYTE *s, *d;
{
	s = skipwh(s);

	if(batch_FLAG && *s == '%' && isnum(s[1]))
	{
		strcpy(d, posparam[tonum(&s[1])]);
		s += 2;
		return s;
	}


	while(*s && !(*s == 0x0d || *s == 0x0a || *s == ' ' || *s == '\t'))
		*d++ = *s++;

	*d = '\0';


	return s;
}

BYTE *scanspl(s, d, c)
BYTE *s, *d;
COUNT c;
{
	s = skipwh(s);
	while(*s && !(*s == 0x0d || *s == 0x0a || *s == ' ' || *s == '\t' || *s == '%' || *s == c))
		*d++ = *s++;

	*d = '\0';
	return s;
}

BYTE *skipwh(s)
BYTE *s;
{
	while(*s && (*s == 0x0d || *s == 0x0a || *s == ' ' || *s == '\t'))
		++s;
	return s;
}


BYTE *scan_seperator(s, d)
BYTE *s, *d;
{
	s = skipwh(s);
	if(*s)
		*d++ = *s++;
	*d = '\0';
	return s;
}


