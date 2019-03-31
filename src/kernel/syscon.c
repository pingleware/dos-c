
/****************************************************************/
/*                                                              */
/*                           syscon.c                           */
/*                                                              */
/*                    Console Device Driver                     */
/*                                                              */
/*                      Copyright (c) 1993                      */
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

/* $Logfile:   D:/dos-c/src/kernel/syscon.c_v  $ */
#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Header:   D:/dos-c/src/kernel/syscon.c_v   1.4   29 May 1996 21:03:30   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/kernel/syscon.c_v  $
 * 
 *    Rev 1.4   29 May 1996 21:03:30   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.3   19 Feb 1996  3:21:36   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.2   01 Sep 1995 17:54:18   patv
 * First GPL release.
 * 
 *    Rev 1.1   30 Jul 1995 20:51:58   patv
 * Eliminated version strings in ipl
 * 
 *    Rev 1.0   02 Jul 1995  8:32:52   patv
 * Initial revision.
 */

#define BREAK_VECTOR    0x1b                    /* break key vector     */
#define CTL_C           0x03                    /* ^C to be returned    */

VOID INRPT break_key();

BOOL break_happened;

WORD 
con_driver (rqptr rp)
{
	REG count, c;
	byteptr cp;

	switch(rp -> r_command)
	{
	case C_INIT:
		kb_init();
		rp -> r_endaddr = device_end();
		rp -> r_nunits = 0;
		setvec(BREAK_VECTOR, break_key);
		break_happened = FALSE;
		return S_DONE;

	case C_OFLUSH:
	case C_OSTAT:
		return S_DONE;

	case C_IFLUSH:
		while(kb_data() >= 0)
			kb_input();
		return S_DONE;

	case C_ISTAT:
		if(kb_data() < 0)
			return S_DONE+S_BUSY;
		return S_DONE;

	case C_OUTPUT:
	case C_OUB:
		count = rp -> r_count;
		cp = rp -> r_trans;
		while(count--)
			con(*cp++);
		return S_DONE;

	case C_INPUT:
		if(break_happened)
		{
			*rp -> r_trans = CTL_C;
			break_happened = FALSE;
			return S_DONE;
		}
		else
			*rp -> r_trans = kb_input();
		rp -> r_count = 1;
		return S_DONE;

	case C_NDREAD:
		if(break_happened)
		{
			rp -> r_ndbyte = CTL_C;
			return S_DONE;
		}
		do
		{
			c = kb_data();
			if(c < 0)
				return S_DONE+S_BUSY;
			else if(c > 0)
			{
				rp -> r_ndbyte = c;
				return S_DONE;
			}
			else
				kb_input();
		}
		while(c == 0);

	default:
		return failure(E_FAILURE);      /* general failure */
	}
}


VOID 
break_handler (void)
{
	break_happened = TRUE;
}


/* "Fast" console handler (int 29h)                                     */
#pragma argsused
VOID INRPT FAR 
int29_handler (int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags)
{
	con(ax & 0xff);
}


