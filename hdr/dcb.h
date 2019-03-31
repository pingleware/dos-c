
/****************************************************************/
/*								*/
/*			      dcb.h				*/
/*								*/
/*		  DOS Device Control Block Structure		*/
/*								*/
/*			 November 20, 1991			*/
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


#ifdef MAIN
# ifdef VERSION_STRINGS
static BYTE *clock_hRcsId = "$Header:   D:/dos-c/hdr/dcb.h_v   1.4   29 May 1996 21:25:20   patv  $";
# endif
#endif

/*
 * $Log:   D:/dos-c/hdr/dcb.h_v  $
 *	
 *	   Rev 1.4   29 May 1996 21:25:20   patv
 *	bug fixes for v0.91a
 *	
 *	   Rev 1.3   19 Feb 1996  3:15:30   patv
 *	Added NLS, int2f and config.sys processing
 *	
 *	   Rev 1.2   01 Sep 1995 17:35:40   patv
 *	First GPL release.
 *	
 *	   Rev 1.1   30 Jul 1995 20:43:48   patv
 *	Eliminated version strings in ipl
 *	
 *	   Rev 1.0   02 Jul 1995 10:39:30   patv
 *	Initial revision.
 */

/* Internal drive parameter block					*/
struct dpb
{
	BYTE	dpb_unit;		/* unit for error reporting	*/
	BYTE	dpb_subunit;		/* the sub-unit for driver	*/
	UWORD	dpb_secsize;		/* sector size			*/
	UBYTE	dpb_clsmask;		/* mask (sectors/cluster-1)	*/
	UBYTE	dpb_shftcnt;		/* log base 2 of cluster size	*/
	UWORD	dpb_fatstrt;		/* FAT start sector		*/
	UBYTE	dpb_fats;		/* # of FAT copies		*/
	UWORD	dpb_dirents;		/* # of dir entries		*/
	UWORD	dpb_data;		/* start of data area		*/
	UWORD	dpb_size;		/* # of clusters+1 on media	*/
	UWORD	dpb_fatsize;		/* # of sectors / FAT		*/
	UWORD	dpb_dirstrt;		/* start sec. of root dir	*/
	struct	dhdr FAR *		/* pointer to device header	*/
		dpb_device;
	UBYTE	dpb_mdb;		/* media descr. byte		*/
	BYTE	dpb_flags;		/* -1 = force MEDIA CHK		*/
	struct	dpb FAR *		/* next dpb in chain		*/
		dpb_next;		/* -1 = end			*/
	UWORD	dpb_cluster;		/* cluster # of first free	*/
					/* -1 if not known		*/
	UWORD	dpb_nfreeclst;		/* number of free clusters	*/
					/* -1 if not known		*/

	/*								*/
	/* Start of DOS-C private areas					*/
	/*								*/
	UBYTE	dpb_clssize;		/* sectors/cluster		*/
	UCOUNT 	dpb_count;		/* number of dpb uses		*/
	BYTE	dpb_path[64];		/* current dir name		*/
};

#define UNKNCLUSTER	0xffff		/* 0xffff = unknow for DOS	*/

