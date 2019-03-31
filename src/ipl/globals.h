
/****************************************************************/
/*								*/
/*			    globals.h				*/
/*								*/
/*	       Global data structures and declarations		*/
/*								*/
/*			 November 9, 1991			*/
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

/* $Logfile:   C:/dos-c/src/ipl/globals.h_v  $ */
#ifdef MAIN
# ifndef IPL
static BYTE *Globals_hRcsId = "$Header:   C:/dos-c/src/ipl/globals.h_v   1.3   29 Aug 1996 13:06:46   patv  $";
# endif
#endif

/*
 * $Log:   C:/dos-c/src/ipl/globals.h_v  $
 *	
 *	   Rev 1.3   29 Aug 1996 13:06:46   patv
 *	Bug fixes for v0.91b
 *	
 *	   Rev 1.2   01 Sep 1995 17:44:38   patv
 *	First GPL release.
 *	
 *	   Rev 1.1   30 Jul 1995 20:48:10   patv
 *	Eliminated version strings in ipl
 *	
 *	   Rev 1.0   02 Jul 1995 10:12:18   patv
 *	Initial revision.
 */

#include "../../hdr/device.h"
#include "../../hdr/mcb.h"
#include "../../hdr/pcb.h"
#include "../../hdr/date.h"
#include "../../hdr/time.h"
#include "../../hdr/fat.h"
#include "../../hdr/fcb.h"
#include "../../hdr/tail.h"
#include "../../hdr/process.h"
#include "../../hdr/dcb.h"
#include "../../hdr/sft.h"
#include "../../hdr/exe.h"
#include "../../hdr/fnode.h"
#include "../../hdr/dirmatch.h"
#include "../../hdr/file.h"
#include "../../hdr/dosnames.h"
#include "../../hdr/clock.h"
#include "../../hdr/kbd.h"
#include "../../hdr/error.h"

#define MAJOR_RELEASE	3
#define MINOR_RELEASE	30
#define REV_NUMBER	0

#define REVISION_MAJOR	1
#define REVISION_MINOR	0

/*									*/
/* Convience switch for maintaining variables in a single location	*/
/*									*/
#ifdef MAIN
#define GLOBAL
#else
#define GLOBAL extern
#endif

/*									*/
/* Convience definitions of TRUE and FALSE				*/
/*									*/
#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

/*									*/
/* Constants and macros							*/
/*									*/
/* Defaults and limits - System wide					*/
#define	NFILES		16		/* number of files in table	*/
#define	NDEVS		4		/* number of supported devices	*/
#define NAMEMAX		64		/* Maximum path for CDS		*/

/* 0 = CON, standard input, can be redirected				*/
/* 1 = CON, standard output, can be redirected				*/
/* 2 = CON, standard error						*/
/* 3 = AUX, auxiliary							*/
/* 4 = PRN, list device							*/
/* 5 = 1st user file ...						*/
#define	STDIN		0
#define	STDOUT		1
#define	STDERR		2
#define	STDAUX		3
#define	STDPRN		4

/* error results returned after asking user				*/
#define	ABORT		-1
#define	RETRY		1
#define	FAIL		0
#define	CONTINUE	2

/* internal error from failure or aborted operation			*/
#define	ERROR		-1
#define	OK		0

/* internal transfer direction flags					*/
#define XFR_READ	1
#define XFR_WRITE	2

#define	RDONLY		0
#define WRONLY		1
#define RDWR		2

/* special ascii code equates						*/
#define	SPCL		0x00
#define	CTL_C		0x03
#define	CTL_F		0x06
#define	BELL		0x07
#define	BS		0x08
#define	HT		0x09
#define	LF		0x0a
#define	CR		0x0d
#define	CTL_Q		0x11
#define	CTL_S		0x13
#define	CTL_Z		0x1a
#define	ESC		0x1b
#define	CTL_BS		0x7f

#define	F3		0x3d
#define LEFT		0x4b
#define RIGHT		0x4d

/* Blockio constants							*/
#define	NUMBUFF	6 /* Number of track buffers -- must be at least 3	*/
#define	DSKWRITE	1		/* dskxfr function parameters	*/
#define	DSKREAD		2

/* FAT cluster special flags						*/
#define FREE			0x000

#define LONG_LAST_CLUSTER	0xFFFF
#define LONG_MASK		0xFFF8
#define LONG_BAD		0xFFF0
#define LAST_CLUSTER		0x0FFF
#define MASK			0xFF8
#define BAD			0xFF0

/*									*/
/* Data structures and unions						*/
/*									*/
/* Sector buffer structure						*/
#define BUFFERSIZE 512
struct	buffer
{
	struct	buffer
		FAR *b_next;		/* form linked list for LRU	*/
	WORD	b_unit;			/* disk for this buffer		*/
	LONG	b_blkno;		/* block for this buffer	*/
	BYTE	b_flag;			/* buffer valid flag		*/
	BYTE	b_update;		/* true if a blockio write has	*/
					/* put data in this buffer,	*/
					/* but the buffer hasn't been	*/
					/* flushed yet.			*/
	BYTE	b_buffer[BUFFERSIZE];	/* 512 byte sectors for now	*/
};

/*									*/
/* External Assembly variables						*/
/*									*/
extern struct dhdr
	FAR clk_dev,		/* Clock device driver			*/
	FAR con_dev,		/* Console device driver		*/
	FAR blk_dev;		/* Block device (Disk) driver		*/
extern UWORD
	ram_top,		/* How much ram in Kbytes		*/
#ifdef I86
	api_sp,			/* api stacks - for context		*/
	api_ss,			/* switching				*/
	usr_ss,			/* user stack segment			*/
	MarkSS,			/* stack highwater segment		*/
#endif
	MarkSP,			/* stack highwater stack pointer	*/
	usr_sp;			/* user stack				*/

extern COUNT *
#ifdef MC68K
	api_sp,			/* api stacks - for context		*/
	MarkSP,			/* stack highwater stack pointer	*/
#endif
	api_tos;		/* API handler stack			*/
extern BYTE
	FAR last;		/* first available byte of ram		*/


/*									*/
/* Global variables							*/
/*									*/
GLOBAL
	iregs far *user_r;
GLOBAL
	seg master_env;		/* Master environment segment		*/

GLOBAL BYTE
	os_major		/* major version number			*/
#ifdef MAIN
	=  MAJOR_RELEASE,
#else
,
#endif
	os_minor		/* minor version number		*/
#ifdef MAIN
	=  MINOR_RELEASE,
#else
,
#endif
	rev_number		/* minor version number		*/
#ifdef MAIN
	=  REV_NUMBER,
#else
,
#endif
	version_flags;		/* minor version number		*/

#ifdef DEBUG
GLOBAL WORD bDumpRdWrParms
# ifdef MAIN
= FALSE;
# else
;
# endif
#endif


GLOBAL BYTE *copyright
#ifdef MAIN
="(C) Copyright 1995,1996\nPasquale J. Villani\nAll Rights Reserved\n";
#else
;
#endif

GLOBAL BYTE *os_release
#ifdef MAIN
# ifdef SHWR
="DOS-C version %d.%02d [Shareware Release].\n\
\n\
May be freely redistributed provided you do not alter the kernel or any\n\
associated files.\n";
# else
="DOS-C version %d.%02d\n\n";
# endif
#else
;
#endif

/* Globally referenced variables				*/
extern	UWORD
	first_mcb;		/* Start of user memory		*/
extern	struct dpb
	FAR *DPBp;		/* First drive Parameter Block	*/
extern	sfttbl
	FAR *sfthead;		/* System File Table head	*/
extern	struct dhdr
	FAR *clock,		/* CLOCK$ device		*/
	FAR *syscon;		/* console device		*/
extern	WORD
	maxbksize;		/* Number of Drives in system	*/
extern	struct buffer
	FAR *firstbuf;    	/* head of buffers linked list	*/
extern	VOID			/* temporary			*/
	FAR *CDSp;		/* Current Directory Structure	*/
extern	fcb
	FAR *FCBp;		/* FCB table pointer		*/
extern	WORD
	nprotfcb;		/* number of protected fcbs	*/
extern	BYTE
	nblkdev,		/* number of block devices	*/
	lastdrive;		/* value of last drive		*/
extern	struct dhdr
	nul_dev;
extern	BYTE	njoined;		/* number of joined devices	*/
extern	WORD
	BootDrive,		/* Drive we came up from		*/
	NumFloppies;		/* How many floppies we have		*/

/* start of uncontrolled variables					*/
GLOBAL seg	cu_psp;			/* current psp segment		*/

GLOBAL iregs	error_regs;		/* registers for dump		*/

GLOBAL WORD
	tsr,			/* true if program is TSR		*/
	break_flg,		/* true if break was detected		*/
	dump_regs,		/* dump registers of bad call		*/
	scr_pos,		/* screen position for bs, ht, etc	*/
	default_drive;		/* default drive for dos		*/

GLOBAL keyboard
	kb_buf;

GLOBAL sfttbl
	basesft;

GLOBAL struct dpb
	blk_devices[NDEVS];

GLOBAL struct f_node
	f_nodes[NFILES];

GLOBAL struct buffer
	FAR *lastbuf,      	/* tail of ditto			*/
	buffers[NUMBUFF];   	/* array of track buffers		*/

GLOBAL iregs
	FAR *ustackp,		/* user stack				*/
	FAR *kstackp;		/* kernel stack				*/

GLOBAL BYTE
	verify_ena,		/* verify enabled flag			*/
	break_ena,		/* break enabled flag			*/
	return_mode,		/* Process termination rets		*/
	return_code,		/*     "        "       "		*/
	mem_access_mode;	/* memory allocation scheme		*/


GLOBAL BYTE
	InDOS,			/* In DOS critical section		*/
	FAR *dta;		/* Disk transfer area (kludge)		*/

/*									*/
/* Function prototypes - automatically generated			*/
/*									*/
#include "proto.h"

/* Process related functions - not under automatic generation.	*/
/* Typically, these are in ".asm" files.			*/
VOID
	INRPT FAR re_entry(),
	INRPT FAR exec_user(),
	INRPT FAR handle_break();
VOID
	enable(),
	disable();

#ifdef PROTO
VOID	FAR set_stack(VOID);
VOID	FAR restore_stack(VOID);
VOID	execrh(request FAR *, struct dhdr FAR *);
VOID	exit(COUNT);
VOID	INRPT FAR handle_break(VOID);
VOID	tmark(VOID);
BOOL	tdelay(LONG);
BYTE	FAR *device_end(VOID);
COUNT	kb_data(VOID);
COUNT	kb_input(VOID);
VOID	setvec(UWORD, VOID (INRPT FAR *)());
COUNT	con(COUNT);
VOID getdirent(BYTE FAR *, struct dirent FAR *);
VOID putdirent(struct dirent FAR *, BYTE FAR *);
#else
VOID	FAR set_stack();
VOID	FAR restore_stack();
VOID	execrh();
VOID	exit();
VOID	INRPT FAR handle_break();
VOID	tmark();
BOOL	tdelay();
BYTE	FAR *device_end();
COUNT	kb_data();
COUNT	kb_input();
VOID	setvec();
COUNT	con();
VOID getdirent();
VOID putdirent();
#endif


/*								*/
/* special word packing prototypes				*/
/*								*/
#ifdef NATIVE
# define getlong(vp, lp) (*(BYTE *)(lp)=*(LONG *)(vp))
# define getword(vp, wp) (*(WORD *)(wp)=*(WORD *)(vp))
# define getbyte(vp, bp) (*(BYTE *)(bp)=*(BYTE *)(vp))
# define fgetlong(vp, lp) (*(LONG FAR *)(lp)=*(LONG FAR *)(vp))
# define fgetword(vp, wp) (*(WORD FAR *)(wp)=*(WORD FAR *)(vp))
# define fgetbyte(vp, bp) (*(BYTE FAR *)(bp)=*(BYTE FAR *)(vp))
# define fputlong(lp, vp) (*(LONG FAR *)(vp)=*(LONG FAR *)(lp))
# define fputword(wp, vp) (*(WORD FAR *)(vp)=*(WORD FAR *)(wp))
# define fputbyte(bp, vp) (*(BYTE FAR *)(vp)=*(BYTE FAR *)(bp))
#else
# ifdef PROTO
VOID getword(VOID *, WORD *);
VOID getbyte(VOID *, BYTE *);
VOID fgetlong(VOID FAR *, LONG FAR *);
VOID fgetword(VOID FAR *, WORD FAR *);
VOID fgetbyte(VOID FAR *, BYTE FAR *);
VOID fputlong(LONG FAR *, VOID FAR *);
VOID fputword(WORD FAR *, VOID FAR *);
VOID fputbyte(BYTE FAR *, VOID FAR *);
# else
VOID getword();
VOID getbyte();
VOID fgetlong();
VOID fgetword();
VOID fgetbyte();
VOID fputlong();
VOID fputword();
VOID fputbyte();
# endif
#endif


/* Special IPL defines							*/
#define KERNSEG 0x00c0
#define upMem(s,c) touc(s,c)
