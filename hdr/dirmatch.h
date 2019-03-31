
/****************************************************************/
/*								*/
/*			    dirmatch.h				*/
/*								*/
/*		 FAT File System Match Data Structure		*/
/*								*/
/*			 January 4, 1992			*/
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


/* $Logfile:   D:/dos-c/hdr/dirmatch.h_v  $ */
#ifdef MAIN
# ifdef VERSION_STRINGS
static BYTE *dirmatch_hRcsId = "$Header:   D:/dos-c/hdr/dirmatch.h_v   1.4   29 May 1996 21:25:18   patv  $";
# endif
#endif

/*
 * $Log:   D:/dos-c/hdr/dirmatch.h_v  $
 *	
 *	   Rev 1.4   29 May 1996 21:25:18   patv
 *	bug fixes for v0.91a
 *	
 *	   Rev 1.3   19 Feb 1996  3:15:34   patv
 *	Added NLS, int2f and config.sys processing
 *	
 *	   Rev 1.2   01 Sep 1995 17:35:40   patv
 *	First GPL release.
 *	
 *	   Rev 1.1   30 Jul 1995 20:43:48   patv
 *	Eliminated version strings in ipl
 *	
 *	   Rev 1.0   02 Jul 1995 10:39:34   patv
 *	Initial revision.
 * 
 *    Rev 1.0   25 May 1993 23:30:26   patv
 * Initial revision.
 * 
 */


typedef struct
{
	BYTE	dm_drive;
	BYTE	dm_name_pat[FNAME_SIZE+FEXT_SIZE];
	BYTE	dm_attr_srch;
	UWORD	dm_entry;
	UWORD	dm_cluster;
	struct
	{
		BYTE		/* directory has been modified	*/
			f_dmod:1;
		BYTE		/* directory is the root	*/
			f_droot:1;
		BYTE		/* fnode is new and needs fill	*/
			f_dnew:1;
		BYTE		/* fnode is assigned to dir	*/
			f_ddir:1;
		BYTE		/* directory is full		*/
			f_dfull:1;
	}	dm_flags;	/* file flags			*/
	UWORD   dm_dirstart;
	BYTE    reserved[1];
	BYTE	dm_attr_fnd;		/* found file attribute		*/
	time	dm_time;		/* file time			*/
	date	dm_date;		/* file date			*/
	LONG	dm_size;		/* file size			*/
	BYTE	dm_name[FNAME_SIZE+FEXT_SIZE+2];	/* file name	*/
} dmatch;

