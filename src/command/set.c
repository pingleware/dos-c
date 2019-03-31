/****************************************************************/
/*								*/
/*			     set.c				*/
/*								*/
/*		      command.com set command			*/
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

/* $Logfile:   C:/dos-c/src/command/set.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/set.c_v  $ 
 * 
 *    Rev 1.2   29 Aug 1996 13:06:56   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:44   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:02:20   patv
 * Initial revision.
 */

static char *RcsId = "$Header:   C:/dos-c/src/command/set.c_v   1.2   29 Aug 1996 13:06:56   patv  $";

#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

extern BYTE *dflt_pr_string;

BOOL set_bat()
{
	BYTE env_var[MAX_CMDLINE];
	BYTE *lp, *p;

	lp = skipwh(tail);
	if(*lp == '\0')
	{
		EnvDump();
		printf("\n");
		return TRUE;
	}

	lp = scanspl(tail, env_var, '=');
	/* capitalized the variable name				*/
	for(p = env_var; *p != '\0'; p++)
		*p = toupper(*p);

	if(*lp != '=')
	{
		error_message(INV_SYNTAX);
		return FALSE;
	}
	else
		++lp;

	if(*lp == '\r' || *lp == '\n')
	{
		/* set env_var in environment to empty			*/
		EnvClearVar(env_var);

		/* Update system PROMPT immediately			*/
		if(strcmp(env_var,"PROMPT") == 0)
			scopy(dflt_pr_string, prompt_string);

	}
	else
	{
		/* Trim trailing newline				*/

		for(p = lp; (*p != '\r') && (*p != '\n'); p++)
			;
		*p = '\0';

		EnvSetVar(env_var, lp);

		/* Update system PROMPT immediately			*/
		if(strcmp(env_var,"PROMPT") == 0)
			scopy(lp, prompt_string);

		/* Update system PATH immediately			*/
		if(strcmp(env_var,"PATH") == 0)
			scopy(lp, path);
	}
	return TRUE;
}
