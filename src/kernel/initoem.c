
/****************************************************************/
/*								*/
/*			    initoem.c				*/
/*								*/
/*		    OEM Initializattion Functions		*/
/*								*/
/*			Copyright (c) 1995			*/
/*		 	Pasquale J. Villani			*/
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
/*								*/
/****************************************************************/


#include "../../hdr/portab.h"
#include "globals.h"

/* $Logfile:   D:/dos-c/src/kernel/initoem.c_v  $ */
#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Header:   D:/dos-c/src/kernel/initoem.c_v   1.3   29 May 1996 21:03:48   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/kernel/initoem.c_v  $
 * 
 *    Rev 1.3   29 May 1996 21:03:48   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.2   19 Feb 1996  3:21:34   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.1   01 Sep 1995 17:54:16   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995  8:31:54   patv
 * Initial revision.
 * 
 */

UWORD 
init_oem (void)
{
	UWORD top_k;

#ifndef __TURBOC__
	_asm
	{
		int	12h
		mov	top_k,ax
	}
#else
	asm {
	int	0x12
	mov	top_k,ax
	}
#endif
	return top_k;
}


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
