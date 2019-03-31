
/****************************************************************/
/*								*/
/*			        cd.c				*/
/*								*/
/*		         DOS "cd" Command 			*/
/*								*/
/*			 November 6, 1991			*/
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


/* $Logfile:   C:/dos-c/src/command/cd.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/cd.c_v  $
 * 
 *    Rev 1.2   29 Aug 1996 13:07:04   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:34   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:01:46   patv
 * Initial revision.
 */



#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

static BYTE *RcsId = "$Header:   C:/dos-c/src/command/cd.c_v   1.2   29 Aug 1996 13:07:04   patv  $";

BOOL cd(argc, argv)
WORD argc;
BYTE *argv[];
{
	COUNT OldDrive, NewDrive = -1;
	BYTE CurDir[MAX_CMDLINE] = "";

	/* Initialize where we are					*/
	OldDrive = DosGetDrive();

	/* Do command line sanity checks				*/
	if(argc > 2)
	{
		error_message(INV_NUM_PARAMS);
		return FALSE;
	}

	/* get command line options and switch to the requested drive	*/
	dosopt("$d*", (BYTE FAR *)tail, &NewDrive, CurDir);
	if(NewDrive < 0)
		NewDrive = default_drive;
	DosSetDrive(NewDrive);

	/* Do pwd function for no parameter case			*/
	if(*CurDir == '\0')
	{
		DosPwd(NewDrive + 1, CurDir);
		printf("%c:\\%s\n\n", 'A' + NewDrive, CurDir);
		DosSetDrive(OldDrive);
		return TRUE;
	}

	/* Otherwise, change the directory, and then switch back to the	*/
	/* old directory.						*/
	if((DosCd((BYTE FAR *)CurDir)) != SUCCESS)
	{
		error_message(INV_DIR);
		DosSetDrive(OldDrive);
		return FALSE;
	}
	else
	{
		DosSetDrive(OldDrive);
		return TRUE;
	}
}


