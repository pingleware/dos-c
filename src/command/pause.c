/****************************************************************/
/*								*/
/*			     pause.c				*/
/*								*/
/*		      command.com pause command			*/
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

/* $Logfile:   C:/dos-c/src/command/pause.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/pause.c_v  $ 
 * 
 *    Rev 1.2   29 Aug 1996 13:06:56   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:40   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:02:12   patv
 * Initial revision.
 */

static char *RcsId = "$Header:   C:/dos-c/src/command/pause.c_v   1.2   29 Aug 1996 13:06:56   patv  $";

#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

BOOL pause_bat()
{
	printf("Press any key to continue . . .\n");
	while(!DosCkKbReady())
		/* This is a location for time sharing */
		;
	DosRdKb();
	return TRUE;
}

