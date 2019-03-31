
/****************************************************************/
/*								*/
/*			    process.h				*/
/*								*/
/*	      DOS exec data structures & declarations		*/
/*								*/
/*			November 23, 1991			*/
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


/* $Logfile:   D:/dos-c/hdr/process.h_v  $ */
#ifdef MAIN
# ifdef VERSION_STRINGS
static BYTE *process_hRcsId = "$Header:   D:/dos-c/hdr/process.h_v   1.4   29 May 1996 21:25:16   patv  $";
# endif
#endif

/*
 * $Log:   D:/dos-c/hdr/process.h_v  $
 *	
 *	   Rev 1.4   29 May 1996 21:25:16   patv
 *	bug fixes for v0.91a
 *	
 *	   Rev 1.3   19 Feb 1996  3:15:32   patv
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

/*  Modes available as first argument to the spawnxx functions. */

#define P_WAIT    0 /* child runs separately, parent waits until exit */
#define P_NOWAIT  1 /* both concurrent -- not implemented */
#define P_OVERLAY 2 /* child replaces parent, parent no longer exists */

typedef struct
{
	union
	{
		struct
		{
			UWORD	load_seg;
			UWORD	reloc;
		} _load;
		struct
		{
			UWORD	env_seg;
			CommandTail FAR	*cmd_line;
			BYTE FAR *fcb_1;
			BYTE FAR *fcb_2;
			BYTE FAR *stack;
			BYTE FAR *start_addr;
		} _exec;
	} ldata;
} exec_blk;

#define	exec	ldata._exec
#define load	ldata._load


typedef struct
{
	UWORD	ps_exit;	/* CP/M-like exit poimt		*/
	UWORD	ps_size;	/* memory size in paragraphs	*/
	BYTE	ps_fill1;	/* single char fill		*/

	/* CP/M-like entry point				*/
	BYTE	ps_farcall;	/* far call opcode		*/
	VOID	(FAR *ps_reentry)(); /* re-entry point		*/
	VOID	(interrupt FAR *ps_isv22)(), /* terminate address */
		(interrupt FAR *ps_isv23)(), /* break address	*/
		(interrupt FAR *ps_isv24)(); /* critical error address */
	UWORD	ps_parent;	/* parent psp segment		*/
	UBYTE	ps_files[20];	/* file table - 0xff is unused	*/
	UWORD	ps_environ;	/* environment paragraph	*/
	BYTE FAR *ps_stack;	/* user stack pointer - int 21	*/
	WORD	ps_maxfiles;	/* maximum open files		*/
	UBYTE FAR *ps_filetab;	/* open file table pointer	*/
	BYTE FAR *ps_prevpsp;	/* previous psp pointer		*/
	BYTE FAR *ps_dta;	/* process dta address		*/
	BYTE	ps_fill2[16];
	BYTE	ps_unix[3];	/* unix style call - 0xcd 0x21 0xcb */
	BYTE	ps_fill3[9];
	union
	{
		struct
		{
			fcb
				_ps_fcb1; /* first command line argument */
		} _u1;
		struct
		{
			BYTE
				fill4[16];
			fcb
				_ps_fcb2; /* second command line argument */
		} _u2;
		struct
		{
			BYTE	fill5[36];
			struct
			{
				BYTE _ps_cmd_count;
				BYTE _ps_cmd[127]; /* command tail	*/
			} _u4;
		} _u3;
	} _u;
} psp;


#define ps_fcb1	_u._u1._ps_fcb1
#define ps_fcb2	_u._u2._ps_fcb2
#define ps_cmd	_u._u3._u4._ps_cmd
#define ps_cmd_count	_u._u3._u4._ps_cmd_count
