/****************************************************************/
/*								*/
/*			      for.c				*/
/*								*/
/*		       command.com for command 			*/
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


/* $Logfile:   C:/dos-c/src/command/for.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/for.c_v  $ 
 * 
 *    Rev 1.2   29 Aug 1996 13:06:52   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:38   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:01:56   patv
 * Initial revision.
 */


#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

static BYTE *RcsId = "$Header:   C:/dos-c/src/command/for.c_v   1.2   29 Aug 1996 13:06:52   patv  $";

extern BOOL batch_FLAG;


BOOL for_bat()
{
	BYTE variable;
	BYTE *dataset;
	BYTE token[MAX_CMDLINE];
	BYTE *lp, *s;
	dmatch dmp;

	lp = scan(tail, token);

	if(*lp == '\0' || *token != '%')
	{
		error_message(INV_SYNTAX);
		return FALSE;
	}
	else
	{
	/* get the loop variable name					*/
		lp++;
		variable = token[1];
	}

	if(*lp == '\0')
	{
		error_message(INV_SYNTAX);
		return FALSE;
	}

	/* check the rest of the syntax: in (....) do		*/
	lp = scan(lp, token);
	if(*lp == '\0')
	{
		error_message(INV_SYNTAX);
		return FALSE;
	}

	for(s = token; *s != '\0'; s++)
		*s = toupper(*s);
	if(strcmp("IN", token) != 0)
	{
		error_message(INV_SYNTAX);
		return FALSE;
	}

	/* get the dataset: (...)				*/
	lp = skipwh(lp);
	if(*lp == '\0' || *lp != '(')
	{
		error_message(INV_SYNTAX);
		return FALSE;
	}
	else
	{
		lp = skipwh(++lp);
		dataset = lp;
		while(*lp && !(*lp == 0x0d || *lp == 0x0a || *lp == ')'))
			lp++;
		*lp++ = '\0';
	}

	if(*lp == '\0')
	{
		error_message(INV_SYNTAX);
		return FALSE;
	}
	else if(*lp == ')')
		++lp;

	lp = scan(lp, token);
	for(s = token; *s != '\0'; s++)
		*s = toupper(*s);

	if(strcmp("DO", token) != 0)
	{
		error_message(INV_SYNTAX);
		return FALSE;
	}

	/* parsing complete, now scan "dataset" and loop for each	*/
	/* element in the set						*/

	lp = skipwh(lp);
	while(*dataset != '\0')
	{
		dataset = scan(dataset, token);
		if(iswild(token))
		{
			DosSetDta((BYTE FAR *)&dmp);
			if(DosFindFirst(D_NORMAL, (BYTE FAR *)token) != SUCCESS)
				return TRUE;

			do
			{
				strcpy(token, dmp.dm_name);
				expandspl(cmd_line, lp, variable, token);
				do_command(strlen(cmd_line));
			}
			while(DosFindNext() == SUCCESS);
		}
		else
		{
			expandspl(cmd_line, lp, variable, token);
			do_command(strlen(cmd_line));
		}
	}
	return TRUE;
}

