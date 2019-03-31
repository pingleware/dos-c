
/****************************************************************/
/*								*/
/*			    blockio.c				*/
/*			      DOS-C				*/
/*								*/
/*	Block cache functions and device driver interface	*/
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
/*								*/
/****************************************************************/


#include "../../hdr/portab.h"
#include "globals.h"

/* $Logfile:   D:/dos-c/src/fs/blockio.c_v  $ */
#ifndef IPL
static BYTE *blockioRcsId = "$Header:   D:/dos-c/src/fs/blockio.c_v   1.3   29 May 1996 21:15:10   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/fs/blockio.c_v  $
 * 
 *    Rev 1.3   29 May 1996 21:15:10   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.2   01 Sep 1995 17:48:46   patv
 * First GPL release.
 * 
 *    Rev 1.1   30 Jul 1995 20:50:28   patv
 * Eliminated version strings in ipl
 * 
 *    Rev 1.0   02 Jul 1995  8:04:06   patv
 * Initial revision.
 */


/************************************************************************/
/*									*/
/*			block cache routines				*/
/*									*/
/************************************************************************/

/*									*/
/* Initialize the buffer structure					*/
/*									*/
VOID 
init_buffers (void)
{
	REG WORD i;

	for (i = 0; i < NUMBUFF; ++i)
	{
		buffers[i].b_flag = FALSE;
		buffers[i].b_update = FALSE;
		if (i < (NUMBUFF - 1))
			buffers[i].b_next = &buffers[i+1];
		else
			buffers[i].b_next = NULL;
	}
	firstbuf = &buffers[0];
	lastbuf  = &buffers[NUMBUFF-1];
}
 

/*									*/
/*	Return the address of a buffer structure containing the		*/
/*	requested block.						*/
/*									*/
/*	returns:							*/
/*		requested block with data				*/
/*	failure:							*/
/*		returns NULL						*/
/*									*/
struct buffer FAR *
getblock (LONG blkno, COUNT dsk)
{
	REG struct buffer FAR *bp;
	REG struct buffer FAR *lbp;
	REG struct buffer FAR *mbp;
	REG WORD imsave;

	/* Search through buffers to see if the required block	*/
	/* is already in a buffer				*/

	bp = firstbuf;
	lbp = NULL;
	mbp = NULL;
	while(bp != NULL)
	{
		if ((bp -> b_flag) && (bp -> b_unit == dsk) 
		     && (bp -> b_blkno == blkno) )
		{
			/* found it -- rearrange LRU links	*/
			if(lbp != NULL)
			{
				lbp -> b_next = bp -> b_next;
				bp -> b_next  = firstbuf;
				firstbuf = bp;
			}
			return(bp);
		}
		else
		{
			mbp = lbp;	/* move along to next buffer */
			lbp = bp;
			bp  = bp -> b_next;
		}
	}
	/* The block we need is not in a buffer, we must make a buffer	*/
	/* available, and fill it with the desired block		*/

	/* detach lru buffer						*/
	if(mbp != NULL)
		mbp -> b_next = NULL;
	lbp -> b_next = firstbuf;
	firstbuf = lbp;
	if(flush1(lbp) && fill(lbp, blkno, dsk)) /* success		*/
		mbp = lbp;
	else
		mbp = NULL;			/* failure		*/
	return (mbp);
}


/*									*/
/*	Mark all buffers for a disk as not valid			*/
/*									*/
VOID 
setinvld (REG COUNT dsk)
{
	REG struct buffer FAR *bp;

	bp = firstbuf;
	while (bp)
	{
		if(bp -> b_unit == dsk)
			bp -> b_flag = FALSE;
		bp = bp -> b_next;
	}
}


/*									*/
/*			Flush all buffers for a disk			*/
/*									*/
/*	returns:							*/
/*		TRUE on success						*/
/*									*/
BOOL 
flush_buffers (REG COUNT dsk)
{
	REG struct buffer FAR *bp;
	REG BOOL ok = TRUE;

	bp = firstbuf;
	while (bp)
	{
		if(bp -> b_unit == dsk)
			if(!flush1(bp))
				ok = FALSE;
		bp = bp -> b_next;
	}
	return ok;
}


/*									*/
/*	Write one disk buffer						*/
/*									*/
BOOL 
flush1 (struct buffer FAR *bp)
{
	REG WORD ok;

	if (bp -> b_flag && bp -> b_update)
		ok = dskxfer(bp -> b_unit, bp -> b_blkno, (VOID FAR *)bp -> b_buffer, DSKWRITE);
	else
		ok = TRUE;
	bp -> b_update = FALSE;		/* even if error, mark not dirty */
	bp -> b_flag &= ok;		/* otherwise system has trouble  */
					/* continuing.			 */
	return(ok);
}


/*									*/
/*	Write all disk buffers						*/
/*									*/
BOOL 
flush (void)
{
	REG struct buffer FAR *bp;
	REG BOOL ok;

	ok = TRUE;
	bp = firstbuf;
	while(bp)
	{
		if (!flush1(bp))
			ok = FALSE;
		bp -> b_flag = FALSE;
		bp = bp -> b_next;
	}
	return(ok);
}


/*									*/
/*	Fill the indicated disk buffer with the current track and sector */
/*									*/
BOOL 
fill (REG struct buffer FAR *bp, LONG blkno, COUNT dsk)
{
	REG WORD ok;

	if(bp -> b_flag && bp -> b_update)
		ok = flush1(bp);
	else
		ok = TRUE;

	if(ok)
		ok = dskxfer(dsk, blkno, (VOID FAR *)bp -> b_buffer, DSKREAD);
	bp -> b_flag = ok;
	bp -> b_update = FALSE;
	bp -> b_blkno = blkno;
	bp -> b_unit = dsk;
	return(ok);
}


/************************************************************************/
/*									*/
/*		Device Driver Interface Functions			*/
/*									*/
/************************************************************************/

/*									*/
/* Transfer a block to/from disk					*/
/*									*/
BOOL 
dskxfer (COUNT dsk, LONG blkno, VOID FAR *buf, COUNT mode)
{
	REG struct dpb *dpbp = &blk_devices[dsk];
	request rq;

	for(;;)
	{
		rq.r_length = sizeof(request);
		rq.r_unit = dpbp -> dpb_subunit;
		rq.r_command =
		 mode == DSKWRITE ?
		  (verify_ena ? C_OUTVFY : C_OUTPUT)
		  : C_INPUT;
		rq.r_status = 0;
		rq.r_meddesc = dpbp -> dpb_mdb;
		rq.r_trans = (BYTE FAR *)buf;
		rq.r_count = 1;
		if(blkno > MAXSHORT)
		{
			rq.r_start = HUGECOUNT;
			rq.r_huge = blkno - 1;
		}
		else
			rq.r_start = blkno - 1;
		execrh((request FAR *)&rq, dpbp -> dpb_device);
		if(!(rq.r_status & S_ERROR) && (rq.r_status & S_DONE))
			break;
		else
		{
		loop:
			switch(block_error(&rq, dpbp -> dpb_unit))
			{
			case ABORT:
			case FAIL:
				return FALSE;

			case RETRY:
				continue;

			case CONTINUE:
				break;

			default:
				goto loop;
			}
		}
	}
	return TRUE;
}


