/****************************************************************/
/*								*/
/*			     type.c				*/
/*								*/
/*		      command.com type command			*/
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

/* $Logfile:   C:/dos-c/src/command/type.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/type.c_v  $ 
 * 
 *    Rev 1.2   29 Aug 1996 13:06:58   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:44   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:02:26   patv
 * Initial revision.
 */

static char *RcsId = "$Header:   C:/dos-c/src/command/type.c_v   1.2   29 Aug 1996 13:06:58   patv  $";

#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

#define CTL_Z 0x1a

BOOL type()
{
	WORD fd;
	BYTE c;
	BYTE fname[MAX_CMDLINE];

	/* parse for options						*/
	dosopt("*", (BYTE FAR *)tail, fname);

	if((fd = DosOpen((BYTE FAR *)fname, 0)) < 0)
	{
		error_mess_str=fname;
		error_message(FILE_NOT_FOUND);
		return FALSE;
	}
	while(DosRead(fd, (BYTE FAR *)&c, 1) == 1)
	{
		if(c == CTL_Z)
			break;
		printf("%c", c);
	}
	DosClose(fd);
	printf("\n");
	return TRUE;
}


