/****************************************************************/
/*								*/
/*			        if.c				*/
/*								*/
/*		       command.com if command 			*/
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

/* $Logfile:   C:/dos-c/src/command/if.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/if.c_v  $ 
 * 
 *    Rev 1.2   29 Aug 1996 13:06:54   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:38   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:02:04   patv
 * Initial revision.
 */

static char *RcsId = "$Header:   C:/dos-c/src/command/if.c_v   1.2   29 Aug 1996 13:06:54   patv  $";

#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

extern BOOL batch_FLAG;

BOOL if_bat()
{
	BYTE *lp, *s;
	BYTE token[MAX_CMDLINE];
	BOOL negate = FALSE;
	BYTE *tokenptr;
	BYTE string1[MAX_CMDLINE];
	BYTE string2[MAX_CMDLINE];

	/* First look for the optional "NOT"				*/
	lp = scan(tail, token);
	for(s = token; *s != '\0'; s++)
		*s = toupper(*s);

	/* If the "NOT" is there, swallow and set negate flag		*/
	if(strcmp("NOT", token) == 0)
	{
		lp = scan(lp, token);
		for(s = token; *s != '\0'; s++)
			*s = toupper(*s);
		negate = TRUE;
	}

	if(strcmp("ERRORLEVEL", token) == 0)
	{
		COUNT errlvl;

		/* Get errorlevel parameter				*/
		lp = scan(lp, token);
		errlvl = atoi(token);

		if((errlvl <= rtn_errlvl && !negate) || (errlvl > rtn_errlvl && negate))
		{
			COUNT len;

			lp = skipwh(lp);
			strncpy(cmd_line, lp, len = strlen(lp));
			do_command(len);
		}
		else
			return FALSE;
		return TRUE;
	}
	else if(strcmp("EXIST", token) == 0)
	{
		BOOL exists;
		COUNT curdrvno;
		COUNT len;
		BYTE LASTDRIVE;
		BYTE drive;

		if((LASTDRIVE = *EnvLookup("LASTDRIVE")) == NULL)
		    LASTDRIVE = 'Z';
		lp = scan(lp,token);
		len = strlen(token);

		/* Check if a drive is prepended			*/
		tokenptr = token;
		drive = toupper(*tokenptr);
		if(len >= 3 && drive >= 'A' && drive <= LASTDRIVE && *(token + 1) == ':')
		{
			COUNT NewDrive = drive - 'A';

			curdrvno = DosGetDrive();
			if(DosSetDrive(NewDrive) < NewDrive)
			{
				DosSetDrive(curdrvno);
				return FALSE;
			}
			tokenptr += 2;
			exists = DosFindFirst(D_DIR, (BYTE FAR *)tokenptr) == SUCCESS;
			DosSetDrive(curdrvno);
		}
		else
			exists = DosFindFirst(D_DIR, (BYTE FAR *)token) == SUCCESS;
		if((!negate && exists) || (negate && !exists))
		{
			lp = skipwh(lp);
			strncpy(cmd_line, lp, len = strlen(lp));
			do_command(len);
			return TRUE;
		}
		else
			return FALSE;
	}
	else
	{
		COUNT len;
		COUNT idx;

		lp = skipwh(tail);
		if(*lp == '\0')
			return FALSE;
		/* see if its a string compare		 		*/
		/* if it is string is now string1			*/
		lp = scanspl(lp, string1, '=');

		for(idx = 0; idx <= 1; idx++)
		{
			if(*lp == '=')
				++lp;
			else
			{
				error_message(INV_SYNTAX);
				return FALSE;
			}
		}

		lp = scan(lp, string2);
		if(*lp == '\0')
		{
			error_message(INV_SYNTAX);
			return FALSE;
		}
		else if(strcmp(string1, string2) == 0)
		{
			/* string1 == string2 execute the command	*/
			lp = skipwh(lp);

			if(*lp == '\0')
			{
				error_message(INV_SYNTAX);
				return FALSE;
			}
			strncpy(cmd_line, lp, len = strlen(lp));
			do_command(len);
			return TRUE;
		}
		else
			return FALSE;
	}
}
