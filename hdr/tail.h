
/****************************************************************/
/*								*/
/*			      tail.h				*/
/*								*/
/*		   Command tail data structures			*/
/*								*/
/*			   July 1, 1993				*/
/*								*/
/*		        Copyright (c) 1995			*/
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


#ifdef MAIN
# ifdef VERSION_STRINGS
static BYTE *tail_hRcsId = "$Header:   D:/dos-c/hdr/tail.h_v   1.4   29 May 1996 21:25:18   patv  $";
# endif
#endif

/*
 * $Log:   D:/dos-c/hdr/tail.h_v  $
 *	
 *	   Rev 1.4   29 May 1996 21:25:18   patv
 *	bug fixes for v0.91a
 *	
 *	   Rev 1.3   19 Feb 1996  3:15:34   patv
 *	Added NLS, int2f and config.sys processing
 *	
 *	   Rev 1.2   01 Sep 1995 17:35:44   patv
 *	First GPL release.
 *	
 *	   Rev 1.1   30 Jul 1995 20:43:50   patv
 *	Eliminated version strings in ipl
 *	
 *	   Rev 1.0   02 Jul 1995 10:39:54   patv
 *	Initial revision.
 */

#ifndef LINESIZE
# define LINESIZE	127
#endif

typedef struct
{
	UBYTE	ctCount;	/* number of bytes returned		*/
	BYTE	ctBuffer[LINESIZE];	/* the buffer itself		*/
} CommandTail;

