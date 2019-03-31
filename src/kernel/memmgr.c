/****************************************************************/
/*                                                              */
/*                          memmgr.c                            */
/*                                                              */
/*               Memory Manager for Core Allocation             */
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


/* $Logfile*/
#ifdef VERSION_STRING
static BYTE *memmgrRcsId = "$Header:   D:/dos-c/src/kernel/memmgr.c_v   1.4   29 May 1996 21:03:34   patv  $";
#endif

/*
 * $Log:   D:/dos-c/src/kernel/memmgr.c_v  $
 * 
 *    Rev 1.4   29 May 1996 21:03:34   patv
 * bug fixes for v0.91a
 * 
 *    Rev 1.3   19 Feb 1996  3:21:36   patv
 * Added NLS, int2f and config.sys processing
 * 
 *    Rev 1.2   01 Sep 1995 17:54:20   patv
 * First GPL release.
 * 
 *    Rev 1.1   30 Jul 1995 20:51:58   patv
 * Eliminated version strings in ipl
 * 
 *    Rev 1.0   02 Jul 1995  8:33:08   patv
 * Initial revision.
 */

VOID mcb_init();        
VOID mcb_print();
VOID show_chain();

/* The following code is 8086 dependant                         */

#ifdef KERNEL
VOID 
mcb_init (mcb FAR *mcbp, UWORD size)
{
	COUNT i;

	mcbp -> m_type = MCB_LAST;
	mcbp -> m_psp = FREE_PSP;
	mcbp -> m_size = size;
	for(i = 0; i < 8; i++)
		mcbp -> m_name[i] = '\0';
	mem_access_mode = FIRST_FIT;
}
#endif

seg 
far2para (VOID FAR *p)
{
	seg u1 = FP_SEG(p);
	offset u2 = FP_OFF(p);
	ULONG phy_addr;

	phy_addr = (((long)u1) << 4) + u2;
	return (phy_addr>>4);
}


seg 
long2para (LONG size)
{
	return ((size + 0x0f)>>4);
}


VOID FAR *
add_far (VOID FAR *fp, ULONG off)
{
	UWORD seg_val;
	UWORD off_val;

	/* Break far pointer into components                            */
	seg_val = FP_SEG(fp);
	off_val = FP_OFF(fp);

	/* add the offset  to the fp's offset part                      */
	off += off_val;

	/* make off_val equal to lower part of new value                */
	off_val = off & 0xffff;

	/* and add top part into seg                                    */
	seg_val += ((off & 0x000f0000l) / 0x10);

	/* and send back the new pointer                                */
	return (VOID FAR *)MK_FP(seg_val, off_val);
}


VOID FAR *
adjust_far (VOID FAR *fp)
{
	ULONG linear;
	UWORD seg_val;
	UWORD off_val;

	/* First, convert the segmented pointer to a linear address     */
	linear = (((ULONG)FP_SEG(fp)) << 4) + FP_OFF(fp);

	/* Break it into segments.                                      */
	seg_val = (UWORD)(linear >> 4);
	off_val = (UWORD)(linear & 0xf);

	/* and return an adddress adjusted to the nearest paragraph     */
	/* boundary.                                                    */
	return MK_FP(seg_val, off_val);
}


#ifdef KERNEL
COUNT 
DosMemAlloc (UWORD size, COUNT mode, seg FAR *para, UWORD FAR *asize)
{
	REG mcb FAR *p;
	mcb FAR *q;
	COUNT i;
	BOOL found;

	/* Initialize                                           */
	p = (mcb FAR *)(MK_FP(first_mcb, 0));

	/* Search through memory blocks                         */
	for(q = (mcb FAR *)0, i = 0, found = FALSE; !found; )
	{
		/* check for corruption                         */
		if(p -> m_type != MCB_NORMAL && p -> m_type != MCB_LAST)
			return DE_MCBDESTRY;

		/* Test if free based on mode rules             */
		switch(mode)
		{
		case LAST_FIT:
		default:
			/* Check for a last fit candidate       */
			if(p -> m_size >= size && p -> m_psp == FREE_PSP)
				/* keep the last know fit       */
				q = p;
			/* not free - bump the pointer          */
			if(p -> m_type != MCB_LAST)
				p = MK_FP(far2para((VOID FAR *)p) + p -> m_size + 1, 0);
			/* was there no room (q == 0)?          */
			else if(p -> m_type == MCB_LAST && q == (mcb FAR *)0)
				return DE_NOMEM;
			/* something was found - continue       */
			else
				found = TRUE;
			break;

		case FIRST_FIT:
			/* Check for a first fit candidate      */
			if(p -> m_size >= size && p -> m_psp == FREE_PSP)
			{
				q = p;
				found = TRUE;
				break;
			}
			/* not free - bump the pointer          */
			if(p -> m_type != MCB_LAST)
				p = MK_FP(far2para((VOID FAR *)p) + p -> m_size + 1, 0);
			/* nothing found till end - no room     */
			else
				return DE_NOMEM;
			break;

		case BEST_FIT:
			/* Check for a best fit candidate       */
			if(p -> m_size >= size && p -> m_psp == FREE_PSP)
			{
				if(i == 0 || p -> m_size < i)
				{
					i = p -> m_size;
					q = p;
				}
			}
			/* not free - bump the pointer          */
			if(p -> m_type != MCB_LAST)
				p = MK_FP(far2para((VOID FAR *)p) + p -> m_size + 1, 0);
			/* was there no room (q == 0)?          */
			else if(p -> m_type == MCB_LAST && q == (mcb FAR *)0)
				return DE_NOMEM;
			/* something was found - continue       */
			else
				found = TRUE;
			break;

		case LARGEST:
			/* Check for a first fit candidate      */
			if((p -> m_psp == FREE_PSP) && (i == 0 || p -> m_size > i))
			{
				size = *asize = i = p -> m_size;
				q = p;
			}
			/* not free - bump the pointer          */
			if(p -> m_type != MCB_LAST)
				p = MK_FP(far2para((VOID FAR *)p) + p -> m_size + 1, 0);
			/* was there no room (q == 0)?          */
			else if(p -> m_type == MCB_LAST && q == (mcb FAR *)0)
				return DE_NOMEM;
			/* something was found - continue       */
			else
				found = TRUE;
			break;
		}
	}
	p = q;
	/* Larger fit case              */
	if(p -> m_size > size)
	{
		if(mode != LAST_FIT)
		{
			q = MK_FP(far2para((VOID FAR *)p) + size + 1, 0);
			/* Always flow m_type up */
			/* on alloc             */
			q -> m_type = p -> m_type;
			p -> m_type = MCB_NORMAL;
			p -> m_psp = cu_psp;
			q -> m_psp = FREE_PSP;
			q -> m_size = p -> m_size - size - 1;
			p -> m_size = size;
			for(i = 0; i < 8; i++)
				p -> m_name[i] = q -> m_name[i] = '\0';
		}
		else
		{
			q = MK_FP(far2para((VOID FAR *)p) + (p -> m_size - size), 0);
			/* Always flow m_type up */
			/* on alloc             */
			q -> m_type = p -> m_type;
			p -> m_type = MCB_NORMAL;
			q -> m_psp = cu_psp;
			p -> m_psp = FREE_PSP;
			p -> m_size = p -> m_size - size - 1;
			q -> m_size = size;
			for(i = 0; i < 8; i++)
				p -> m_name[i] = q -> m_name[i] = '\0';
		}
		/* Found - return good          */
		*para = far2para((VOID FAR *)(mode == LAST_FIT ? (VOID FAR *)q : (VOID FAR *)p));
		return SUCCESS;
	}
	/* Exact fit case               */
	else if(p -> m_size == size)
	{
		p -> m_psp = cu_psp;
		for(i = 0; i < 8; i++)
			p -> m_name[i] = '\0';
		/* Found - return good          */
		*para = far2para((VOID FAR *)(BYTE FAR *)p);
		return SUCCESS;
	}
	else
		return DE_MCBDESTRY;
}


seg 
DosMemLargest (seg FAR *size)
{
	REG mcb FAR *p;
	mcb FAR *q;
	COUNT found;

	/* Initialize                                           */
	p = (mcb FAR *)(MK_FP(first_mcb, 0));

	/* Search through memory blocks                         */
	for(q = (mcb FAR *)0, *size = 0, found = FALSE; !found; )
	{
		/* check for corruption                         */
		if(p -> m_type != MCB_NORMAL && p -> m_type != MCB_LAST)
			return DE_MCBDESTRY;

		/* Test for largest fit/available               */
		if((p -> m_psp == FREE_PSP) && (p -> m_size > *size))
		{
			*size = p -> m_size;
			q = p;
		}
		/* not free - bump the pointer          */
		if(p -> m_type != MCB_LAST)
			p = MK_FP(far2para((VOID FAR *)p) + p -> m_size + 1, 0);
		/* was there no room (q == 0)?          */
		else if(p -> m_type == MCB_LAST && q == (mcb FAR *)0)
			return DE_NOMEM;
		/* something was found - continue       */
		else
			found = TRUE;
	}
	if( q != 0)
		return SUCCESS;
	else
		return DE_NOMEM;
}


COUNT 
DosMemFree (UWORD para)
{
	REG mcb FAR *p, FAR *q;
	COUNT i;

	/* Initialize                                           */
	p = (mcb FAR *)(MK_FP(para, 0));

	/* check for corruption                         */
	if(p -> m_type != MCB_NORMAL && p -> m_type != MCB_LAST)
		return DE_INVLDMCB;

	/* Mark the mcb as free so that we can later    */
	/* merge with other surrounding free mcb's      */
	p -> m_psp = FREE_PSP;
	for(i = 0; i < 8; i++)
		p -> m_name[i] = '\0';

	/* Now merge free blocks                        */
	for(p = (mcb FAR *)(MK_FP(first_mcb, 0)); p -> m_type != MCB_LAST; p = q)
	{
		/* make q a pointer to the next block   */
		q = MK_FP(far2para((VOID FAR *)p) + p -> m_size + 1, 0);
		/* and test for corruption              */
		if(q -> m_type != MCB_NORMAL && q -> m_type != MCB_LAST)
			return DE_MCBDESTRY;
		if(p -> m_psp != FREE_PSP)
			continue;

		/* test if next is free - if so merge   */
		if(q -> m_psp == FREE_PSP)
		{
			/* Always flow type down on free */
			p -> m_type = q -> m_type;
			p -> m_size += q -> m_size + 1;
			/* and make pointers the same   */
			/* since the next free is now   */
			/* this block                   */
			q = p;
		}
	}
	return SUCCESS;
}

COUNT 
DosMemChange (UWORD para, UWORD size, UWORD *maxSize)
{
	REG mcb FAR *p, FAR *q;
	REG COUNT i;


	/* Initialize                                                   */
	p = (mcb FAR *)(MK_FP(--para, 0));

	/* check for corruption                                         */
	if(p -> m_type != MCB_NORMAL && p -> m_type != MCB_LAST)
		return DE_MCBDESTRY;

	/* check for wrong allocation                                   */
	if(size > p -> m_size)
	{
		REG UCOUNT delta;

		/* make q a pointer to the next block                   */
		q = MK_FP(far2para((VOID FAR *)p) + p -> m_size + 1, 0);

	       /* check for corruption                                         */
	       if(q -> m_type != MCB_NORMAL && q -> m_type != MCB_LAST)
		  return DE_MCBDESTRY;
	       
		/* if next mcb is not free, error no memory             */
		if(q -> m_psp != FREE_PSP)
		{        
			if (maxSize) *maxSize = p -> m_size;
			return DE_NOMEM;
		}
		/* reduce the size of q and add difference to p         */
		/* but check that q is big enough first                 */
		delta = size - p -> m_size;
		 
		if(q -> m_size < delta)
		{        
			
			if (maxSize) *maxSize = p -> m_size + q -> m_size;
			return DE_NOMEM;
		}
		
		q -> m_size -= delta;
		p -> m_size += delta;

		/* Now go back and adjust q, we'll make p new q         */
		p = MK_FP(far2para((VOID FAR *)q) + delta, 0);
		p -> m_type = q -> m_type;
		p -> m_psp  = q -> m_psp;
		p -> m_size = q -> m_size;
		
		for(i = 0; i < 8; i++)
		    p -> m_name[i] = q -> m_name[i];

		/* and finished                                         */
		return SUCCESS;
	}

	/* else, shrink it down                                         */
	else if(size < p -> m_size)
	{
		/* make q a pointer to the new next block               */
		q = MK_FP(far2para((VOID FAR *)p) + size + 1, 0);

		/* reduce the size of p and add difference to q         */
		q -> m_type = p -> m_type;
		q -> m_size = p -> m_size - size - 1;
		p -> m_size = size;

		/* Make certian the old psp is not last (if it was)     */
		p -> m_type = MCB_NORMAL;

		/* Mark the mcb as free so that we can later    */
		/* merge with other surrounding free mcb's      */
		q -> m_psp = FREE_PSP;
		for(i = 0; i < 8; i++)
			q -> m_name[i] = '\0';

		/* now free it so that we have a complete block */
		return DosMemFree(far2para((VOID FAR *)q));
	}

	/* otherwise, its a no-op                                       */
	else
		return SUCCESS;
}

COUNT 
DosMemCheck (void)
{
	REG mcb FAR *p;

	/* Initialize                                           */
	p = (mcb FAR *)(MK_FP(first_mcb, 0));

	/* Search through memory blocks                         */
	for( ; ; )
	{
		/* check for corruption                         */
		if(p -> m_type != MCB_NORMAL && p -> m_type != MCB_LAST)
			return DE_MCBDESTRY;

		/* not corrupted - if last we're OK!            */
		if(p -> m_type == MCB_LAST)
			return SUCCESS;

		/* not corrupted - but not end, bump the pointer */
		else if(p -> m_type != MCB_LAST)
			p = MK_FP(far2para((VOID FAR *)p) + p -> m_size + 1, 0);

		/* totally lost - bad exit                      */
		else
			return DE_MCBDESTRY;
	}
}

COUNT 
FreeProcessMem (UWORD ps)
{
	mcb FAR *p;

	/* Initialize                                           */
	p = (mcb FAR *)(MK_FP(first_mcb, 0));

	/* Search through memory blocks                         */
	for( ; ; )
	{
		/* check for corruption                         */
		if(p -> m_type != MCB_NORMAL && p -> m_type != MCB_LAST)
			return DE_MCBDESTRY;

		if(p -> m_psp == ps) 
			DosMemFree( FP_SEG(p) );

		/* not corrupted - if last we're OK!            */
		if(p -> m_type == MCB_LAST) break;
		p = MK_FP(far2para((VOID FAR *)p) + p -> m_size + 1, 0);

	}
	return SUCCESS;
}

COUNT
DosGetLargestBlock(UWORD FAR *block)
{
	UWORD sz = 0;
	mcb FAR *p;
	*block = sz;

	/* Initialize                                           */
	p = (mcb FAR *)(MK_FP(first_mcb, 0));

	/* Search through memory blocks                         */
	for( ; ; )
	{
		/* check for corruption                         */
		if(p -> m_type != MCB_NORMAL && p -> m_type != MCB_LAST)
			return DE_MCBDESTRY;

		if (p -> m_psp == FREE_PSP && p -> m_size > sz)
		    sz = p -> m_size;

		/* not corrupted - if last we're OK!            */
		if(p -> m_type == MCB_LAST) break;
		p = MK_FP(far2para((VOID FAR *)p) + p -> m_size + 1, 0);

	}
	*block = sz;
	return SUCCESS;
}

# ifdef DEBUG
VOID 
show_chain (void)
{
	mcb FAR *p = (mcb FAR *)(MK_FP(first_mcb, 0));

	for(;;)
	{
		mcb_print(p);
		if(p -> m_type == MCB_LAST || p -> m_type != MCB_NORMAL)
			return;
		else
			p = (mcb FAR *)(MK_FP(far2para((VOID FAR *)p)+p -> m_size+1,0));
	}
}


VOID 
mcb_print (mcb FAR *mcbp)
{
	static BYTE buff[9];
	VOID _fmemcpy();

	_fmemcpy((BYTE FAR *)buff, (BYTE FAR *)(mcbp -> m_name), 8);
	buff[8] = '\0';
	printf("%04x:%04x -> |%s| m_type = 0x%02x; m_psp = 0x%04x; m_size = 0x%04x\n",
		FP_SEG(mcbp),
		FP_OFF(mcbp),
		*buff == '\0' ? "*NO-ID*" : buff,
		mcbp -> m_type,
		mcbp -> m_psp,
		mcbp -> m_size);
}


VOID _fmemcpy(BYTE FAR *d, BYTE FAR *s, REG COUNT n)
{
	while(n--)
	      *d++ = *s++;

}
# endif
#endif

