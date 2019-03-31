
/****************************************************************/
/*                                                              */
/*                          ioctl.c                             */
/*                                                              */
/*                    DOS/NT ioctl system call                  */
/*                                                              */
/*                      Copyright (c) 1995                      */
/*                      Pasquale J. Villani                     */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of DOS-C.                                  */
/*                                                              */
/* DOS-C is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU General Public License  */
/* as published by the Free Software Foundation; either version */
/* 2, or (at your option) any later version.                    */
/*                                                              */
/* DOS-C is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See    */
/* the GNU General Public License for more details.             */
/*                                                              */
/* You should have received a copy of the GNU General Public    */
/* License along with DOS-C; see the file COPYING.  If not,     */
/* write to the Free Software Foundation, 675 Mass Ave,         */
/* Cambridge, MA 02139, USA.                                    */
/****************************************************************/

#include "../../hdr/portab.h"
#include "globals.h"

/* $Logfile:   D:/dos-c/src/kernel/ioctl.c_v  $ */
#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Header:   D:/dos-c/src/kernel/ioctl.c_v   1.3   29 May 1996 21:03:30   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/kernel/ioctl.c_v  $
 * 
 *    Rev 1.3   29 May 1996 21:03:30   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.2   19 Feb 1996  3:21:34   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.1   01 Sep 1995 17:54:16   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995  8:32:04   patv
 * Initial revision.
 */

#ifdef PROTO
sft FAR *get_sft(COUNT);
#else
sft FAR *get_sft();
#endif

COUNT 
DosDevIOctl (iregs FAR *r, COUNT FAR *err)
{
	sft FAR *s;
	COUNT hndl = r -> BX;

	/* Test that the handle is valid                */
	if(hndl < 0)
	{
		*err = DE_INVLDHNDL;
		return 0;
	}

	/* Get the SFT block that contains the SFT      */
	if((s = get_sft(hndl)) == (sft FAR *)-1)
	{
		*err = DE_INVLDHNDL;
		return 0;
	}

	switch(r -> AL)
	{
	case 0:
		r -> DX = s -> sft_flags;
		break;

	case 1:
		s -> sft_flags =
		  (s -> sft_dev -> dh_attr & ~SFT_MASK) |  0x80 | (r -> DX & SFT_MASK);
		break;

	/* default is do-nothing                        */
	}
	*err = SUCCESS;
	return 0;
}



static sft FAR *
get_sft (COUNT hndl)
{
	psp FAR *p = MK_FP(cu_psp,0);
	COUNT sys_idx;
	sfttbl FAR *sp;

	/* Get the SFT block that contains the SFT      */
	sys_idx = p -> ps_filetab[hndl];
	for(sp = sfthead; sp != (sfttbl FAR *)-1; sp = sp -> sftt_next)
	{
		if(sys_idx < sp -> sftt_count)
			break;
		else
			sys_idx -= sp -> sftt_count;
	}

	/* If not found, return an error                */
	if(sp == (sfttbl FAR *)-1)
		return (sft FAR *)-1;

	/* finally, point to the right entry            */
	return (sft FAR *)&(sp -> sftt_table[sys_idx]);
}


