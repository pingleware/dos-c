
/****************************************************************/
/*								*/
/*			    error.h				*/
/*								*/
/*		     DOS/NT error return codes 			*/
/*								*/
/*			 December 1, 1991			*/
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


/* $Logfile:   D:/dos-c/hdr/error.h_v  $ */
#ifdef MAIN
# ifdef VERSION_STRINGS
static BYTE *error_hRcsId = "$Header:   D:/dos-c/hdr/error.h_v   1.4   29 May 1996 21:25:18   patv  $";
# endif
#endif

/*
 * $Log:   D:/dos-c/hdr/error.h_v  $
 *	
 *	   Rev 1.4   29 May 1996 21:25:18   patv
 *	bug fixes for v0.91a
 *	
 *	   Rev 1.3   19 Feb 1996  3:15:28   patv
 *	Added NLS, int2f and config.sys processing
 *	
 *	   Rev 1.2   01 Sep 1995 17:35:38   patv
 *	First GPL release.
 *	
 *	   Rev 1.1   30 Jul 1995 20:42:28   patv
 *	fixed ipl
 *	
 *	   Rev 1.0   02 Jul 1995 10:39:36   patv
 *	Initial revision.
 */

/* Internal system error returns					*/
#define SUCCESS		0		/* Function was successful	*/
#define	DE_INVLDFUNC	-1		/* Invalid function number	*/
#define	DE_FILENOTFND	-2		/* File not found		*/
#define	DE_PATHNOTFND	-3		/* Path not found		*/
#define	DE_TOOMANY	-4		/* Too many open files		*/
#define	DE_ACCESS	-5		/* Access denied		*/
#define	DE_INVLDHNDL	-6		/* Invalid handle		*/
#define	DE_MCBDESTRY	-7		/* Memory control blocks shot	*/
#define	DE_NOMEM	-8		/* Insufficient memory		*/
#define	DE_INVLDMCB	-9		/* Invalid memory control block	*/
#define	DE_INVLDENV	-10		/* Invalid enviornement		*/
#define	DE_INVLDFMT	-11		/* Invalid format		*/
#define	DE_INVLDACC	-12		/* Invalid access		*/
#define	DE_INVLDDATA	-13		/* Inavalid data		*/
#define	DE_INVLDDRV	-14		/* Invalid drive		*/
#define	DE_RMVCUDIR	-15		/* Attempt remove current dir	*/
#define	DE_DEVICE	-16		/* Not same device		*/
#define	DE_NFILES	-17		/* No more files		*/
#define DE_BLKINVLD	-18		/* invalid block		*/
#define DE_SEEK		-27		/* error on file seek		*/
#define DE_HNDLDSKFULL	-39		/* handle disk full (?)		*/


