/****************************************************************/
/*								*/
/*			     ren.c				*/
/*								*/
/*		      command.com ren command			*/
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

/* $Logfile:   C:/dos-c/src/command/ren.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/ren.c_v  $ 
 * 
 *    Rev 1.2   29 Aug 1996 13:07:00   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:32   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:01:42   patv
 * Initial revision.
 */

static char *RcsId = "$Header:   C:/dos-c/src/command/ren.c_v   1.2   29 Aug 1996 13:07:00   patv  $";

#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

extern BOOL batch_FLAG;

BOOL ren()
{
	BYTE *lp, *llp;
	BYTE old_name[MAX_CMDLINE], new_name[MAX_CMDLINE];

	lp = scan(tail, old_name);

	if(*lp == '\r' || *lp == '\n')
	{
		error_message(INV_NUM_PARAMS);
		return FALSE;
	}

	lp = scan(lp, new_name);

	llp = skipwh(lp);
	if(*llp != '\0')
	{
		error_message(INV_NUM_PARAMS);
		return FALSE;
	}

	if(DosRename((BYTE FAR *)old_name, (BYTE FAR *)new_name) == SUCCESS)
		return TRUE;
	else
	{
		error_message(DUP_FILE);
		return FALSE;
	}
}


