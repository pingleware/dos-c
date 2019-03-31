/****************************************************************/
/*								*/
/*			     dosopt.c				*/
/*								*/
/*	     DOS Reusable Command Line Parsing Function		*/
/*								*/
/*			    June 2, 1993			*/
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


/* $Logfile:   C:/dos-c/src/misc/dosopt.c_v  $ */

/*
 * $Log:   C:/dos-c/src/misc/dosopt.c_v  $ 
 *	
 *	   Rev 1.2   29 Aug 1996 13:07:36   patv
 *	Bug fixes for v0.91b
 *	
 *	   Rev 1.1   01 Sep 1995 18:11:00   patv
 *	First GPL release.
 *	
 *	   Rev 1.0   02 Jul 1995 11:04:46   patv
 *	Initial revision.
 */


#include "../../hdr/portab.h"

#ifdef PROTO
BOOL iswild(BYTE *);
BYTE FAR *skipwh(BYTE FAR *);
COUNT iseol(COUNT);
COUNT issep(COUNT);
BOOL iswhite(COUNT);
COUNT tolower(COUNT);
#else
BOOL iswild();
BYTE FAR *skipwh();
COUNT iseol();
COUNT issep();
BOOL iswhite();
COUNT tolower();
#endif


BYTE optswitch = '/';

COUNT dosopt(fmt, s, args)
BYTE *fmt;
BYTE FAR *s;
VOID *args;
{
	VOID **argp = &args, **startp;
	BYTE *start, FAR *skipwh();
	COUNT ret_count = 0;
	BYTE FAR *p, FAR *q;

	while(*fmt != '\0' && !iseol(*s))
	{
		switch(*fmt)
		{
		case '*':		/* The anything but option case	*/
			s = skipwh(s);
			if(!issep(*s))
				++ret_count;
			while(!issep(*s))
				*((BYTE *)(*argp))++ = *s++;
			*((BYTE *)(*argp))++ = '\0';
			++((BYTE **)argp);
			++fmt;
			break;

		case '$':
			++fmt;
			switch(*fmt)
			{
			case 'd':
				s = skipwh(s);
				if(s[1] == ':')
				{
					*((COUNT *)(*argp)) = ((tolower(*s)) - 'a');
					s = &s[2];
					++ret_count;
				}
				else
					*((COUNT *)(*argp))++ = -1;
				++((COUNT **)argp);
				++fmt;
				break;

			case 'p':
				s = skipwh(s);
				q = (BYTE FAR *)0;
				for(p = s; !issep(*p); ++p)
				{
					if(*p == '/' || *p == '\\')
						q = p;
				}
				if(q != (BYTE FAR *)0)
				{
					COUNT n = 0;

					do
					{
						*((BYTE *)(*argp))++ = *s++;
						++n;
					}
					while(s <= q);
					if(n > 1 && (((BYTE *)(*argp))[-1] == '\\' || ((BYTE *)(*argp))[-1] == '/'))
						((BYTE *)(*argp))[-1] = '\0';
					++ret_count;
				}
				*((BYTE *)(*argp))++ = '\0';
				++((BYTE **)argp);
				++fmt;
				break;
			}
			break;

		case '[':
			start = fmt;
			startp = argp;
			s = skipwh(s);
			if(*s == optswitch)
			{
				++s;
				++fmt;
				while(*fmt != ']')
				{
					if(*fmt == ':')
					{
						++fmt;
						continue;
					}
					if(*fmt++ == tolower(*s))
					{
						if(*fmt == ':')
						{
							++s, ++s;
							while(!issep(*s))
								*((BYTE *)(*argp))++ = *s++;
							++fmt;
						}
						else
						{
							*((COUNT *)(*argp)) ^= 1;
							++s;
						}

						while(*fmt++ != ']')
							++((COUNT **)argp);
						--fmt;
						++ret_count;
					}
					else
						++((COUNT **)argp);
				}
				if(*(fmt + 1) == '+')
				{
					argp = startp;
					fmt = start;
				}
			}
			else
				while(!issep(*s))
					++s;
			break;

		default:
			break;
		}
	}
	return ret_count;
}


BOOL iswild(s)
BYTE *s;
{
	for( ; *s != '\0'; ++s)
		if(*s == '?' || *s == '*')
			return TRUE;
	return FALSE;
}


static COUNT tolower(c)
COUNT c;
{
	static BYTE *alpha[2] =
	{
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		"abcdefghijklmnopqrstuvwxyz"
	};
	COUNT idx;

	for(idx = 0; alpha[0][idx] != '\0'; ++idx)
	{
		if(alpha[0][idx] == c)
			return alpha[1][idx];
	}
	return c;
}


static BOOL iswhite(c)
COUNT c;
{
	return (c == ' ' || c == '\t');
}


static BYTE FAR *skipwh(s)
BYTE FAR *s;
{
	while(iswhite(*s))
		++s;
	return s;
}


static COUNT iseol(c)
COUNT c;
{
	return(c == '\0' || c == '\r' || c == '\n');
}


COUNT issep(c)
COUNT c;
{
	return(iseol(c) || iswhite(c) || c == optswitch);
}

