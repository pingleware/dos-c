/****************************************************************/
/*								*/
/*			      home.c				*/
/*								*/
/*		   DOS Find Home Directory Functions		*/
/*								*/
/*			  August 2, 1993			*/
/*								*/
/*			Copyright (c) 1993			*/
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


/* $Logfile:   C:/dos-c/src/misc/home.c_v  $ */
static char *RcsId = "$Header:   C:/dos-c/src/misc/home.c_v   1.2   29 Aug 1996 13:07:36   patv  $";

/*
 * $Log:   C:/dos-c/src/misc/home.c_v  $
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
#include "../../hdr/device.h"
#include "../../hdr/date.h"
#include "../../hdr/time.h"
#include "../../hdr/tail.h"
#include "../../hdr/fcb.h"
#include "../../hdr/process.h"
#include "proto.h"

#define SUCCESS 0

static COUNT home_driveno;
static BYTE home[67];

VOID FindHome()
{
	/* Get the current drive, so we can switch back.       		*/
	home_driveno = DosGetDrive();

	/* Get the current directory. Note that the DOS system call	*/
	/* does not return drive or leading '\', so we need to add	*/
	/* them.							*/
	home[0] = '\\';
	DosPwd(home_driveno + 1, (BYTE FAR *)&home[1]);
}


BOOL GoHome()
{
	DosSetDrive(home_driveno);
	if((DosCd((BYTE FAR *)home)) != SUCCESS)
		return FALSE;
	return TRUE;
}

