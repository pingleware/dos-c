
/****************************************************************/
/*								*/
/*			     error.c				*/
/*								*/
/*		 Main Kernel Error Handler Functions		*/
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

#include "../../hdr/portab.h"

/* $Logfile:   D:/dos-c/src/fs/error.c_v  $ */
#ifndef IPL
static BYTE *errorRcsId = "$Header:   D:/dos-c/src/fs/error.c_v   1.3   29 May 1996 21:15:10   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/fs/error.c_v  $
 * 
 *    Rev 1.3   29 May 1996 21:15:10   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.2   01 Sep 1995 17:48:46   patv
 * First GPL release.
 * 
 *    Rev 1.1   30 Jul 1995 20:50:26   patv
 * Eliminated version strings in ipl
 * 
 *    Rev 1.0   02 Jul 1995  8:06:14   patv
 * Initial revision.
 */

#include "globals.h"


/* error registers					*/
VOID 
dump (void)
{
	printf("Register Dump [AH = %02x CS:IP = %04x:%04x]\n",
		error_regs.AH,
		error_regs.CS,
		error_regs.IP);
	printf("AX:%04x BX:%04x CX:%04x DX:%04x\n",
		error_regs.AX,
		error_regs.BX,
		error_regs.CX,
		error_regs.DX);
	printf("SI:%04x DI:%04x DS:%04x ES:%04x\n",
		error_regs.SI,
		error_regs.DI,
		error_regs.DS,
		error_regs.ES);
}


/* issue a panic message for corrupted data structures		*/
VOID 
panic (BYTE *s)
{
	printf("\nPANIC: %s\nSystem halted\n", s);
	exit(-1);
}


#ifdef IPL
/* issue an internal error message				*/
VOID 
fatal (BYTE *err_msg)
{
	printf("\nInternal IPL error - %s\nSystem halted\n", err_msg);
	exit(-1);
}
#else
/* issue an internal error message				*/
VOID 
fatal (BYTE *err_msg)
{
	printf("\nInternal kernel error - %s\nSystem halted\n", err_msg);
	exit(-1);
}
#endif


/* Abort, retry or fail for character devices			*/
COUNT char_error(rq, devname)
request *rq;
BYTE *devname;
{
	BYTE *s, a;
	
	switch(rq -> r_status & S_MASK)
	{
	case E_WRPRT:
		s = "Write Protect";
		break;

	case E_UNIT:
		s = "Unknown Unit";
		break;

	case E_NOTRDY:
		s = "Device Not Ready";
		break;

	case E_CMD:
		s = "Unknown Command";
		break;

	case E_CRC:
		s = "Crc Error";
		break;

	case E_LENGTH:
		s = "Bad Length";
		break;

	case E_SEEK:
		s = "Seek Error";
		break;

	case E_MEDIA:
		s = "Unknown Media";
		break;

	case E_NOTFND:
		s = "Sector Not Found";
		break;

	case E_PAPER:
		s = "No Paper";
		break;

	case E_WRITE:
		s = "Write Fault";
		break;

	case E_READ:
		s = "Read Fault";
		break;

	case E_FAILURE:
	default:
		s = "General Failure";
		break;
	}
loop:
	printf("\n\n\n%s error on %s %c\nAbort, Retry, Fail or Continue? ",
		s, devname);
	sto(a = _sti());
	printf("\n");
	switch(a)
	{
	case 'A':
	case 'a':
		return ABORT;

	case 'R':
	case 'r':
		return RETRY;

	case 'F':
	case 'f':
		return FAIL;

	case 'C':
	case 'c':
		return CONTINUE;

	default:
		goto loop;
	}
}


/* Abort, retry or fail for block devices			*/
COUNT block_error(rq, drive)
request *rq;
COUNT drive;
{
	BYTE *s, a;
	
	switch(rq -> r_status & S_MASK)
	{
	case E_WRPRT:
		s = "Write Protect";
		break;

	case E_UNIT:
		s = "Unknown Unit";
		break;

	case E_NOTRDY:
		s = "Device Not Ready";
		break;

	case E_CMD:
		s = "Unknown Command";
		break;

	case E_CRC:
		s = "Crc Error";
		break;

	case E_LENGTH:
		s = "Bad Length";
		break;

	case E_SEEK:
		s = "Seek Error";
		break;

	case E_MEDIA:
		s = "Unknown Media";
		break;

	case E_NOTFND:
		s = "Sector Not Found";
		break;

	case E_PAPER:
		s = "No Paper";
		break;

	case E_WRITE:
		s = "Write Fault";
		break;

	case E_READ:
		s = "Read Fault";
		break;

	case E_FAILURE:
	default:
		s = "General Failure";
		break;
	}
loop:
	printf("\n\n\n%s error on drive %c\nAbort, Retry, Fail or Continue? ",
		s, drive + 'A');
	sto(a = _sti());
	printf("\n");
	switch(a)
	{
	case 'A':
	case 'a':
		return ABORT;

	case 'R':
	case 'r':
		return RETRY;

	case 'F':
	case 'f':
		return FAIL;

	case 'C':
	case 'c':
		return CONTINUE;

	default:
		goto loop;
	}
}


