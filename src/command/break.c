
/****************************************************************/
/*								*/
/*			      break.c				*/
/*								*/
/*		     command.com "break" Command 		*/
/*								*/
/*			 December 16, 1991			*/
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


/* $Logfile:   C:/dos-c/src/command/break.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/break.c_v  $
 * 
 *    Rev 1.2   29 Aug 1996 13:07:00   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:30   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:01:34   patv
 * Initial revision.
 */

#include "../../hdr/portab.h"
#include "../../hdr/error.h"
#include "globals.h"
#include "proto.h"

static BYTE *RcsId = "$Header:   C:/dos-c/src/command/break.c_v   1.2   29 Aug 1996 13:07:00   patv  $";

extern BYTE *tail;

BOOL cmd_break()
{
	BYTE state[] = "OFF";
	BYTE break_str[MAX_CMDLINE] = "";
	BOOL mode = FALSE;
	COUNT index = 0;

	dosopt("*", (BYTE FAR *)tail, break_str);

	if(*break_str != '\0')
	{
		if(strlen(break_str) > 3)
		{
			error_message(ON_OFF);
			return FALSE;
		}
		while(break_str[index] != '\0')
		{
			break_str[index] = toupper(break_str[index]);
			++index;
		}
		if(strcmp(break_str, "ON") == 0)
			mode = TRUE;
		else if(strcmp(break_str, "OFF") == 0)
			mode = FALSE;
		else
		{
			error_message(ON_OFF);
			return FALSE;
		}
		DosSetBreak(mode);
	}
	/* check for current state */

	if(DosGetBreak())
		strcpy(state,"ON");

	printf("\nBREAK is %s\n", state);
	return TRUE;
}

