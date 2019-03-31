/****************************************************************/
/*								*/
/*			      env.c				*/
/*								*/
/*		    command.com Environment Support 		*/
/*								*/
/*			  August 9, 1991			*/
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


/* $Logfile:   C:/dos-c/src/command/env.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/env.c_v  $ 
 * 
 *    Rev 1.2   29 Aug 1996 13:06:52   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:36   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:01:54   patv
 * Initial revision.
 */


#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

static BYTE *RcsId = "$Header:   C:/dos-c/src/command/env.c_v   1.2   29 Aug 1996 13:06:52   patv  $";

COUNT EnvSizeUp()
{
	COUNT Size;
	BYTE FAR *s;

	for(Size = 0, s = env; *s != '\0'; )
	{
		while(*s != '\0')
		{
			++s;
			++Size;
		}
		++s;
		++Size;
	}
	return Size;
}


BOOL EnvAlloc(size)
COUNT size;
{
	COUNT klicks;
	BOOL err;
	UWORD seg;

	klicks = (size + PARASIZE - 1)/PARASIZE;
	seg = DosAllocMem(klicks, (BOOL FAR *)&err);
	if(!err)
	{
		BYTE FAR *s, FAR *p;

		p = MK_FP(seg, 0);
		for(s = env; *s != '\0'; )
		{

			fstrncpy(p, s, MAX_CMDLINE);
			while(*s != '\0')
				++s;
			while(*p != '\0')
				++p;
			++s;
			++p;
		}
		*p = '\0';
		env = MK_FP(seg, 0);
		return TRUE;
	}
	else
		return FALSE;
}


BOOL EnvFree(s)
BYTE FAR *s;
{
	UWORD seg;
	BOOL err;

	seg = FP_SEG(s);
	DosFreeMem(seg, (BOOL FAR *)err);
	return !err;
}


BYTE *EnvLookup(name)
BYTE *name;
{
	BYTE FAR *s;
	BYTE namebuf[MAX_CMDLINE], *p;

	strcpy(namebuf, name);
	for(p = namebuf; *p != '\0'; p++)
		*p = toupper(*p);

	for(s = env; *s != '\0'; )
	{
		static BYTE buf[MAX_CMDLINE];
		BYTE pattern[MAX_CMDLINE], *lp;

		fstrncpy((BYTE FAR *)buf, s, MAX_CMDLINE);
		lp = scanspl(buf, pattern, '=');
		++lp;
		if(strcmp(pattern, namebuf) == 0)
			return lp;
		while(*s != '\0')
			++s;
		++s;
	}
	return (BYTE *)0;
}


BOOL EnvClearVar(name)
BYTE *name;
{
	BYTE buf[MAX_CMDLINE];
	BYTE test_name[MAX_CMDLINE];
	BYTE FAR *s, FAR *p;

	for(p = env; *p != '\0'; )
	{
		fstrncpy((BYTE FAR *)buf, p, MAX_CMDLINE);
		scanspl(buf, test_name, '=');

		/* if they compare you have the variable to delete	*/
		if(strcmp(test_name, name) == 0)
		{
			/* scan forward til start of next string 	*/
			/* start copying until the end			*/
			s = p;
			while(*s != '\0')
				s++;
			s++;
			while(1)
			{
				while(*s != '\0')
					*p++ = *s++;
				*p++ = *s++;
				if(*s == '\0')
					break;
			}
			*p = '\0';
			/* Update system PATH immediately			*/
			if(strcmp(name,"PATH") == 0)
				scopy(dflt_path_string, path);
			return TRUE;
		}
		while(*p != '\0')
			++p;
		++p;
	}
	/* never found the variable to clear				*/
	return FALSE;
}


BOOL EnvSetVar(name, value)
BYTE *name;
BYTE *value;
{
	BYTE FAR *s;
	BYTE namebuf[MAX_CMDLINE], *p;
	COUNT old_size = 0;

	strcpy(namebuf, name);

	/* clear current value, then set new value			*/
	EnvClearVar(name);

	/* Get to the end of the environment and add the new string (or	*/
	/* the replacement string) to the end.				*/
	for(s = env; *s != '\0'; )
	{
		while(*s != '\0')
		{
			++old_size;
			++s;
		}
		++s;
		++old_size;
	}

	/* Build the new environment string.				*/
	strcat(namebuf, "=");
	strcat(namebuf, value);

	/* Let's test to see if we have room.				*/
	if(EnvSize < (old_size + strlen(namebuf) + 1))
		return FALSE;

	/* copy the string to the end (as determined above)		*/
	fstrncpy(s, (BYTE FAR *)namebuf, MAX_CMDLINE);

	/* Now terminate the environment				*/
	for( ; *s != '\0'; ++s)
		;
	*++s = '\0';

	return TRUE;
}


BOOL EnvDump()
{
	BYTE FAR *s;

	for(s = env; *s != '\0'; )
	{
		static BYTE buf[MAX_CMDLINE];

		fstrncpy((BYTE FAR *)buf, s, MAX_CMDLINE);
		printf("%s\n",(buf));
		while(*s != '\0')
			++s;
		++s;
	}
	return TRUE;
}
