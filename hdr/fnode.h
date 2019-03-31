
/****************************************************************/
/*								*/
/*			     fnode.h				*/
/*								*/
/*		Internal File Node for FAT File System		*/
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


/* $Logfile:   D:/dos-c/hdr/fnode.h_v  $ */
#ifdef MAIN
# ifdef VERSION_STRINGS
static BYTE *fnode_hRcsId = "$Header:   D:/dos-c/hdr/fnode.h_v   1.4   29 May 1996 21:25:16   patv  $";
# endif
#endif

/*
 * $Log:   D:/dos-c/hdr/fnode.h_v  $
 *	
 *	   Rev 1.4   29 May 1996 21:25:16   patv
 *	bug fixes for v0.91a
 *	
 *	   Rev 1.3   19 Feb 1996  3:15:32   patv
 *	Added NLS, int2f and config.sys processing
 *	
 *	   Rev 1.2   01 Sep 1995 17:35:42   patv
 *	First GPL release.
 *	
 *	   Rev 1.1   30 Jul 1995 20:43:48   patv
 *	Eliminated version strings in ipl
 *	
 *	   Rev 1.0   02 Jul 1995 10:39:44   patv
 *	Initial revision.
 */


struct f_node
{
	UWORD	f_count;	/* number of uses of this file	*/
	COUNT	f_mode;		/* read, write, read-write, etc	*/
	struct
	{
		BOOL		/* directory has been modified	*/
			f_dmod:1;
		BOOL		/* directory is the root	*/
			f_droot:1;
		BOOL		/* fnode is new and needs fill	*/
			f_dnew:1;
		BOOL		/* fnode is assigned to dir	*/
			f_ddir:1;
		BOOL		/* directory is full		*/
			f_dfull:1;
	}	f_flags;	/* file flags			*/
	struct dirent		/* this file's dir entry image	*/
		f_dir;
	ULONG	f_diroff;	/* offset of the dir entry	*/
	UWORD	f_dirstart;	/* the starting cluster of dir	*/
				/* when dir is not root		*/
	struct dpb *		/* the block device for file	*/
		f_dpb;
	ULONG	f_dsize;	/* file size (for directories)	*/
	ULONG	f_offset;	/* byte offset for next op	*/
	ULONG	f_highwater;	/* the largest offset ever	*/
	UWORD	f_back;		/* the cluster we were at	*/
	UWORD	f_cluster;	/* the cluster we are at	*/
	UWORD	f_sector;	/* the sector in the cluster	*/
	UWORD	f_boff;		/* the byte in the cluster	*/
};

