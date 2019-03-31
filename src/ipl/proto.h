
/****************************************************************/
/*								*/
/*			    proto.h				*/
/*								*/
/*		     Global Function Prototypes			*/
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

/* $Logfile:   C:/dos-c/src/ipl/proto.h_v  $ */
#ifdef MAIN
# ifndef IPL
static BYTE *Proto_hRcsId = "$Header:   C:/dos-c/src/ipl/proto.h_v   1.4   29 Aug 1996 13:06:46   patv  $";
# endif
#endif

/*
 * $Log:   C:/dos-c/src/ipl/proto.h_v  $
 *	
 *	   Rev 1.4   29 Aug 1996 13:06:46   patv
 *	Bug fixes for v0.91b
 *	
 *	   Rev 1.3   19 Feb 1996  3:18:20   patv
 *	Added NLS, int2f and config.sys processing
 *	
 *	   Rev 1.2   01 Sep 1995 17:44:42   patv
 *	First GPL release.
 *	
 *	   Rev 1.1   30 Jul 1995 20:48:10   patv
 *	Eliminated version strings in ipl
 *	
 *	   Rev 1.0   02 Jul 1995 10:12:16   patv
 *	Initial revision.
 */

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* FCBFNS.C */
VOID DosOutputString _P((BYTE FAR *s));
int DosCharInputEcho _P((VOID));
VOID DosDirectCosoleIO _P((iregs FAR *r));
VOID DosCharOutput _P((COUNT c));

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* MAIN.C */
VOID main _P((void));
VOID init_device _P((struct dhdr FAR *dhp));
struct dhdr FAR *link_dhdr _P((struct dhdr FAR FAR *lp, struct dhdr FAR *dhp));
COUNT prompt _P((COUNT argc, BYTE *argv[]));
COUNT put_prompt _P((BYTE *fmt));
struct table *lookup _P((struct table *p, BYTE *token));
COUNT unknown _P((void));
COUNT ver _P((void));
COUNT help _P((void));
COUNT type _P((COUNT argc, BYTE *argv[]));
COUNT dir _P((COUNT argc, BYTE *argv[]));

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* MISC.C */
VOID scopy _P((REG BYTE *s, REG BYTE *d));
VOID fscopy _P((REG BYTE FAR *s, REG BYTE FAR FAR *d));
VOID fsncopy _P((BYTE FAR *s, BYTE FAR FAR *d, REG COUNT n));
VOID bcopy _P((REG BYTE *s, REG BYTE *d, REG COUNT n));
VOID fbcopy _P((REG VOID FAR *s, REG VOID FAR FAR *d, REG COUNT n));
BYTE *skipwh _P((BYTE *s));
BYTE *scan _P((BYTE *s, BYTE *d));
BYTE *scan_seperator _P((BYTE *s, BYTE *d));

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* FATDIR.C */
struct f_node FAR *dir_open _P((BYTE FAR *dirname));
COUNT dir_read _P((REG struct f_node FAR *fnp));
VOID dir_close _P((REG struct f_node FAR *fnp));


#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* BLOCKIO.C */
VOID init_buffers _P((void));
struct buffer FAR *getblock _P((LONG blkno, COUNT dsk));
VOID setinvld _P((REG COUNT dsk));
BOOL flush_buffers _P((REG COUNT dsk));
BOOL flush1 _P((struct buffer FAR *bp));
BOOL flush _P((void));
BOOL fill _P((REG struct buffer FAR *bp, LONG blkno, COUNT dsk));
BOOL dskxfer _P((COUNT dsk, LONG blkno, VOID FAR *buf, COUNT mode));

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* SYSCON.C */
WORD con_driver _P((rqptr rp));
VOID break_handler _P((void));

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* ERROR.C */
VOID dump _P((void));
VOID panic _P((BYTE *s));
VOID fatal _P((BYTE *err_msg));
VOID fatal _P((BYTE *err_msg));
COUNT char_error _P((request *rq, BYTE *devname));
COUNT block_error _P((request *rq, COUNT drive));

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* FATTAB.C */
UCOUNT link_fat _P((struct dpb *dpbp, UCOUNT Cluster1, REG UCOUNT Cluster2));
UCOUNT link_fat16 _P((struct dpb *dpbp, UCOUNT Cluster1, UCOUNT Cluster2));
UCOUNT link_fat12 _P((struct dpb *dpbp, UCOUNT Cluster1, UCOUNT Cluster2));
UWORD next_cluster _P((struct dpb *dpbp, REG UCOUNT ClusterNum));
UWORD next_cl16 _P((struct dpb *dpbp, REG UCOUNT ClusterNum));
UWORD next_cl12 _P((struct dpb *dpbp, REG UCOUNT ClusterNum));

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* FATFS.C */
COUNT dos_open _P((BYTE FAR *path, COUNT flag));
BOOL fcmp _P((BYTE FAR *s1, BYTE FAR FAR *s2, COUNT n));
BOOL fcmp_wild _P((BYTE FAR *s1, BYTE FAR FAR *s2, COUNT n));
VOID touc _P((BYTE FAR *s, COUNT n));
COUNT dos_close _P((COUNT fd));
COUNT dos_creat _P((BYTE FAR *path, COUNT attrib));
COUNT dos_delete _P((BYTE FAR *path));
COUNT dos_rmdir _P((BYTE FAR *path));
COUNT dos_rename _P((BYTE FAR *path1, BYTE FAR FAR *path2));
date dos_getdate _P((void));
time dos_gettime _P((void));
BOOL dos_getftime _P((COUNT fd, date FAR *dp, time FAR *tp));
BOOL dos_setftime _P((COUNT fd, date FAR *dp, time FAR *tp));
COUNT dos_mkdir _P((BYTE FAR *dir));
BOOL last_link _P((struct f_node FAR *fnp));
COUNT map_cluster _P((struct f_node FAR *, COUNT));
COUNT dos_read _P((COUNT fd, VOID FAR *buffer, UCOUNT count));
COUNT dos_write _P((COUNT fd, VOID FAR *buffer, UCOUNT count));
LONG dos_lseek _P((COUNT fd, LONG foffset, COUNT origin));
UWORD dos_free _P((struct dpb *dpbp));
VOID dos_pwd _P((struct dpb *dpbp, BYTE FAR *s));
VOID trim_path _P((BYTE FAR *s));
COUNT dos_cd _P((struct dpb *dpbp, BYTE FAR *s));
struct f_node FAR *get_f_node _P((VOID));
VOID release_f_node _P((struct f_node FAR *fnp));
VOID dos_setdta _P((BYTE FAR *newdta));
COUNT dos_findfirst _P((UCOUNT attr, BYTE FAR *name));
COUNT dos_findnext _P((void));
COUNT dos_getfattr _P((BYTE FAR *name, UWORD FAR *attrp));
COUNT dos_setfattr _P((BYTE FAR *name, UWORD FAR *attrp));
COUNT media_check _P((REG struct dpb *dpbp));
struct f_node FAR *xlt_fd _P((COUNT fd));
COUNT xlt_fnp _P((struct f_node FAR *fnp));
struct dhdr FAR *select_unit _P((COUNT drive));

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* STRINGS.C */
COUNT strlen _P((REG BYTE *s));
COUNT fstrlen _P((REG BYTE FAR *s));
VOID strcpy _P((REG BYTE *d, REG BYTE *s));
VOID strncpy _P((REG BYTE *d, REG BYTE *s, COUNT l));
VOID strcat _P((REG BYTE *d, REG BYTE *s));
COUNT strcmp _P((REG BYTE *d, REG BYTE *s));
COUNT strncmp _P((REG BYTE *d, REG BYTE *s, COUNT l));
VOID fstrncpy _P((REG BYTE FAR *d, REG BYTE FAR FAR *s, COUNT l));
COUNT tolower _P((COUNT c));
COUNT toupper _P((COUNT c));

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* CHARIO.C */
VOID INRPT FAR handle_break(void);
VOID sto(COUNT c);
VOID mod_sto(REG UCOUNT c);
VOID destr_bs(void);
UCOUNT _sti(void);
VOID con_hold(void);
BOOL con_break(void);
BOOL KbdBusy(void);
VOID KbdFlush(void);
VOID sti(keyboard FAR *kp);

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* MEMMGR.C */
VOID mcb_init _P((mcb FAR *mcbp, seg size));
seg far2para _P((VOID FAR *p));
seg long2para _P((LONG size));
VOID FAR *add_far _P((VOID FAR *fp, ULONG off));
VOID FAR *adjust_far _P((VOID FAR *fp));
COUNT DosMemAlloc _P((seg size, COUNT mode, seg FAR *para, UWORD FAR *asize));
seg DosMemLargest _P((seg FAR *size));
COUNT DosMemFree _P((seg para));
COUNT DosMemChange _P((seg para, seg size));
COUNT DosMemCheck _P((void));
VOID show_chain _P((void));
VOID mcb_print _P((mcb FAR *mcbp));
VOID _fmemcpy _P((BYTE FAR *d, BYTE FAR *s, REG COUNT n));

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* DSK.C */
COUNT blk_driver _P((rqptr rp));

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* DOSFNS.C */
BOOL check_break _P((void));
UCOUNT DosRead _P((COUNT hndl, UCOUNT n, BYTE FAR *bp, COUNT FAR *err));
UCOUNT DosWrite _P((COUNT hndl, UCOUNT n, BYTE FAR *bp, COUNT FAR *err));
COUNT DosSeek _P((COUNT hndl, LONG new_pos, COUNT mode, ULONG *set_pos));
COUNT DosCreat _P((BYTE FAR *fname, COUNT attrib));
COUNT DosOpen _P((BYTE FAR *fname, COUNT mode));
COUNT DosClose _P((COUNT hndl));
VOID DosGetFree _P((COUNT drive, COUNT FAR *spc, COUNT FAR *navc, COUNT FAR *bps, COUNT FAR *nc));
COUNT DosGetCuDir _P((COUNT drive, BYTE FAR *s));
COUNT DosChangeDir _P((BYTE FAR *s));
COUNT DosFindFirst _P((UCOUNT attr, BYTE FAR *name));
COUNT DosFindNext _P((void));
COUNT DosGetFtime _P((COUNT hndl, date FAR *dp, time FAR *tp));
COUNT DosSetFtime _P((COUNT hndl, date FAR *dp, time FAR *tp));
COUNT DosGetFattr _P((BYTE FAR *name, UWORD FAR *attrp));
COUNT DosSetFattr _P((BYTE FAR *name, UWORD FAR *attrp));

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* PRF.C */
VOID put_console _P((COUNT c));
WORD printf _P((CONST BYTE *fmt, ...));
WORD sprintf _P((BYTE *buff, CONST BYTE *fmt, ...));

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* SYSPACK.C */
VOID getlong _P((REG VOID *vp, LONG *lp));
VOID getword _P((REG VOID *vp, WORD *wp));
VOID getbyte _P((VOID *vp, BYTE *bp));
VOID fgetword _P((REG VOID FAR *vp, WORD FAR *wp));
VOID fgetlong _P((REG VOID FAR *vp, LONG FAR *lp));
VOID fgetbyte _P((VOID FAR *vp, BYTE FAR *bp));
VOID fputlong _P((LONG FAR *lp, VOID FAR *vp));
VOID fputword _P((WORD FAR *wp, VOID FAR *vp));
VOID fputbyte _P((BYTE FAR *bp, VOID FAR *vp));
VOID getdirent _P((BYTE FAR *vp, struct dirent FAR *dp));
VOID putdirent _P((struct dirent FAR *dp, BYTE FAR *vp));

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* TASK.C */
COUNT ChildEnv(exec_blk FAR *exp, UWORD *pChildEnvSeg);
VOID new_psp(psp FAR *p, int psize);
VOID return_user(void);
COUNT DosExec(COUNT mode, exec_blk FAR *ep, BYTE FAR *lp);

#undef _P
#if defined(__STDC__) || defined(__cplusplus) || defined (__BORLANDC__)
# define _P(s) s
#else
# define _P(s) ()
#endif


/* ipl.asm */
VOID MarkStack(VOID);

