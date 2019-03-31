
/****************************************************************/
/*								*/
/*			      sft.h				*/
/*			      DOS-C				*/
/*								*/
/*		   DOS System File Table Structure		*/
/*								*/
/*		     Copyright (c) 1995, 1996			*/
/*		   	Pasquale J. Villani			*/
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


/* $Logfile:   D:/dos-c/hdr/sft.h_v  $ */
#ifdef MAIN
# ifdef VERSION_STRINGS
static BYTE *sft_hRcsId = "$Header:   D:/dos-c/hdr/sft.h_v   1.4   29 May 1996 21:25:18   patv  $";
# endif
#endif

/*
 * $Log:   D:/dos-c/hdr/sft.h_v  $
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
 *	   Rev 1.0   02 Jul 1995 10:39:52   patv
 *	Initial revision.
 */

#define SFTMAX	128

/* Handle Definition entry						*/
typedef struct
{
	WORD	sft_count;	/* reference count			*/
	WORD	sft_mode;	/* open mode - see below		*/
	BYTE	sft_attrib;	/* file attribute - dir style		*/
	WORD	sft_flags;	/* flags - see below			*/
	union
	{
		struct	dpb FAR *
			_sft_dcb;	/* The device control block	*/
		struct dhdr FAR *
			_sft_dev;	/* device driver for char dev	*/
	} sft_dcb_or_dev;
	WORD	sft_stclust;	/* Starting cluster			*/
	time	sft_time;	/* File time				*/
	date	sft_date;	/* File date				*/
	LONG	sft_size;	/* File size				*/
	LONG	sft_posit;	/* Current file position		*/
	WORD	sft_relclust;	/* File relative cluster		*/
	WORD	sft_cuclust;	/* File current cluster			*/
	WORD	sft_dirdlust;	/* Sector containing cluster		*/
	BYTE	sft_diridx;	/* directory index			*/
	BYTE	sft_name[11];	/* dir style file name			*/
	BYTE	FAR *
		sft_bshare;	/* backward link of file sharing sft	*/
	WORD	sft_mach;	/* machine number - network apps	*/
	WORD	sft_psp;	/* owner psp				*/
	WORD	sft_shroff;	/* Sharing offset			*/
	WORD	sft_status;	/* this sft status			*/
} sft;

/* SFT Table header definition						*/
typedef struct _sftheader
{
	struct _sfttbl FAR *		/* link to next table in list	*/
		sftt_next;
	WORD	sftt_count;		/* # of handle definition	*/
					/* entries, this table		*/
} sftheader;

/* System File Definition List						*/
typedef struct _sfttbl
{
	struct _sfttbl FAR *		/* link to next table in list	*/
		sftt_next;
	WORD	sftt_count;		/* # of handle definition	*/
					/* entries, this table		*/
	sft	sftt_table[SFTMAX];	/* The array of sft for block	*/
} sfttbl;

/* defines for sft use							*/
#define SFT_MASK	0x007f		/* splits device data		*/

/* mode bits								*/
#define	SFT_MFCB	0x8000		/* entry is for fcb		*/
#define SFT_MDENYNONE	0x0040		/* sharing bits			*/
#define SFT_MDENYREAD	0x0030		/*     "      "			*/
#define SFT_MDENYWRITE	0x0020		/*     "      "			*/
#define SFT_MEXCLUSIVE	0x0010		/*     "      "			*/
#define SFT_NOINHERIT	0x0080		/* inhibit inherting of file	*/
#define SFT_NETFCB	0x0070		/* networked fcb		*/
#define	SFT_MSHAREMASK	0x0070		/* mask to isolate shared bits	*/
#define	SFT_MRDWR	0x0002		/* read/write bit		*/
#define	SFT_MWRITE	0x0001		/* write bit			*/
#define SFT_MREAD	0x0000		/* ~ write bit			*/
#define SFT_OMASK	0x00f3		/* valid open mask		*/

/* flag bits								*/

/* the following bit is for redirection					*/
#define SFT_FSHARED	0x8000		/* Networked access		*/

/* the following entry differntiates char & block access		*/
#define SFT_FDEVICE	0x0080		/* device entry			*/

/* the following bits are file (block) unique				*/
#define SFT_FDATE	0x4000		/* File date set		*/
#define SFT_FDIRTY	0x0040		/* File has been written to	*/
#define SFT_FDMASK	0x003f		/* File mask for drive no	*/

/* the following bits are device (char) unique				*/
#define SFT_FIOCTL	0x4000		/* IOCTL support - device	*/
#define SFT_FEOF	0x0040		/* device eof			*/
#define SFT_FBINARY	0x0020		/* device binary mode		*/
#define SFT_FSPECIAL	0x0010		/* int 29 support		*/
#define SFT_FCLOCK	0x0008		/* device is clock		*/
#define SFT_FNUL	0x0004		/* device is nul		*/
#define SFT_FSTDOUT	0x0002		/* device is stdout		*/
#define SFT_FSTDIN	0x0001		/* device is stdin		*/

/* Covienence defines							*/
#define	sft_dcb		sft_dcb_or_dev._sft_dcb
#define sft_dev         sft_dcb_or_dev._sft_dev

