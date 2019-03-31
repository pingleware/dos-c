
/****************************************************************/
/*								*/
/*			    proto.h				*/
/*								*/
/*		     Global Function Prototypes			*/
/*								*/
/*		     Copyright (c) 1995, 1996			*/
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

/* $Logfile:   D:/dos-c/src/kernel/proto.h_v  $ */
#ifdef MAIN
# ifdef VERSION_STRINGS
static BYTE *Proto_hRcsId = "$Header:   D:/dos-c/src/kernel/proto.h_v   1.4   29 May 1996 21:03:40   patv  $";
# endif
#endif

/*
 * $Log:   D:/dos-c/src/kernel/proto.h_v  $
 *
 *   Rev 1.4   29 May 1996 21:03:40   patv
 *bug fixes for v0.91a
 *
 *   Rev 1.3   19 Feb 1996  3:23:06   patv
 *Added NLS, int2f and config.sys processing
 *
 *   Rev 1.2   01 Sep 1995 17:54:26   patv
 *First GPL release.
 *
 *   Rev 1.1   30 Jul 1995 20:51:58   patv
 *Eliminated version strings in ipl
 *
 *   Rev 1.0   05 Jul 1995 11:32:16   patv
 *Initial revision.
 */


/* blockio.c */
VOID init_buffers(void);
struct buffer FAR *getblock(LONG blkno, COUNT dsk);
VOID setinvld(REG COUNT dsk);
BOOL flush_buffers(REG COUNT dsk);
BOOL flush1(struct buffer FAR *bp);
BOOL flush(void);
BOOL fill(REG struct buffer FAR *bp, LONG blkno, COUNT dsk);
BOOL dskxfer(COUNT dsk, LONG blkno, VOID FAR *buf, COUNT mode);

/* chario.c */
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

/* config.c */
VOID PreConfig(VOID);
VOID DoConfig(VOID);
VOID PostConfig(VOID);

/* dosfns.c */
BOOL check_break(void);
UCOUNT DosRead(COUNT hndl, UCOUNT n, BYTE FAR *bp, COUNT FAR *err);
UCOUNT DosWrite(COUNT hndl, UCOUNT n, BYTE FAR *bp, COUNT FAR *err);
COUNT DosSeek(COUNT hndl, LONG new_pos, COUNT mode, ULONG *set_pos);
COUNT DosCreat(BYTE FAR *fname, COUNT attrib);
COUNT CloneHandle(COUNT hndl);
COUNT DosDup(COUNT Handle);
COUNT DosForceDup(COUNT OldHandle, COUNT NewHandle);
COUNT DosOpen(BYTE FAR *fname, COUNT mode);
COUNT DosClose(COUNT hndl);
VOID DosGetFree(COUNT drive, COUNT FAR *spc, COUNT FAR *navc, COUNT FAR *bps, COUNT FAR *nc);
COUNT DosGetCuDir(COUNT drive, BYTE FAR *s);
COUNT DosChangeDir(BYTE FAR *s);
COUNT DosFindFirst(UCOUNT attr, BYTE FAR *name);
COUNT DosFindNext(void);
COUNT DosGetFtime(COUNT hndl, date FAR *dp, time FAR *tp);
COUNT DosSetFtime(COUNT hndl, date FAR *dp, time FAR *tp);
COUNT DosGetFattr(BYTE FAR *name, UWORD FAR *attrp);
COUNT DosSetFattr(BYTE FAR *name, UWORD FAR *attrp);

/* dosnames.c */
COUNT DosNames(BYTE FAR *lpszFileName, struct dosnames FAR *lpDosname);
BOOL IsDevice(BYTE *FileName);

/* dsk.c */
COUNT blk_driver(rqptr rp);

/* error.c */
VOID dump(void);
VOID panic(BYTE *s);
VOID fatal(BYTE *err_msg);
COUNT char_error(request *rq, BYTE *devname);
COUNT block_error(request *rq, COUNT drive);

/* fatdir.c */
struct f_node FAR *dir_open(BYTE FAR *dirname);
COUNT dir_read(REG struct f_node FAR *fnp);
COUNT dir_write(REG struct f_node FAR *fnp);
VOID dir_close(REG struct f_node FAR *fnp);
COUNT dos_findfirst(UCOUNT attr, BYTE FAR *name);
COUNT dos_findnext(void);

/* fatfs.c */
COUNT dos_open(BYTE FAR *path, COUNT flag);
BOOL fcmp(BYTE FAR *s1, BYTE FAR *s2, COUNT n);
BOOL fcmp_wild(BYTE FAR *s1, BYTE FAR *s2, COUNT n);
VOID touc(BYTE FAR *s, COUNT n);
COUNT dos_close(COUNT fd);
COUNT dos_creat(BYTE FAR *path, COUNT attrib);
COUNT dos_delete(BYTE FAR *path);
COUNT dos_rmdir(BYTE FAR *path);
COUNT dos_rename(BYTE FAR *path1, BYTE FAR *path2);
date dos_getdate(void);
time dos_gettime(void);
BOOL dos_getftime(COUNT fd, date FAR *dp, time FAR *tp);
BOOL dos_setftime(COUNT fd, date FAR *dp, time FAR *tp);
LONG dos_getcufsize(COUNT fd);
LONG dos_getfsize(COUNT fd);
BOOL dos_setfsize(COUNT fd, LONG size);
COUNT dos_mkdir(BYTE FAR *dir);
BOOL last_link(struct f_node FAR *fnp);
COUNT map_cluster(REG struct f_node FAR *fnp, COUNT mode);
UCOUNT rdwrblock(COUNT fd, VOID FAR *buffer, UCOUNT count, COUNT mode, COUNT *err);
COUNT dos_read(COUNT fd, VOID FAR *buffer, UCOUNT count);
COUNT dos_write(COUNT fd, VOID FAR *buffer, UCOUNT count);
LONG dos_lseek(COUNT fd, LONG foffset, COUNT origin);
UWORD dos_free(struct dpb *dpbp);
VOID dos_pwd(struct dpb *dpbp, BYTE FAR *s);
VOID trim_path(BYTE FAR *s);
COUNT dos_cd(struct dpb *dpbp, BYTE FAR *s);
struct f_node FAR *get_f_node(void);
VOID release_f_node(struct f_node FAR *fnp);
VOID dos_setdta(BYTE FAR *newdta);
COUNT dos_getfattr(BYTE FAR *name, UWORD FAR *attrp);
COUNT dos_setfattr(BYTE FAR *name, UWORD FAR *attrp);
COUNT media_check(REG struct dpb *dpbp);
struct f_node FAR *xlt_fd(COUNT fd);
COUNT xlt_fnp(struct f_node FAR *fnp);
struct dhdr FAR *select_unit(COUNT drive);

/* fattab.c */
UCOUNT link_fat(struct dpb *dpbp, UCOUNT Cluster1, REG UCOUNT Cluster2);
UCOUNT link_fat16(struct dpb *dpbp, UCOUNT Cluster1, UCOUNT Cluster2);
UCOUNT link_fat12(struct dpb *dpbp, UCOUNT Cluster1, UCOUNT Cluster2);
UWORD next_cluster(struct dpb *dpbp, REG UCOUNT ClusterNum);
UWORD next_cl16(struct dpb *dpbp, REG UCOUNT ClusterNum);
UWORD next_cl12(struct dpb *dpbp, REG UCOUNT ClusterNum);

/* fcbfns.c */
VOID DosOutputString(BYTE FAR *s);
int DosCharInputEcho(VOID);
int DosCharInput(VOID);
VOID DosDirectConsoleIO(iregs FAR *r);
VOID DosCharOutput(COUNT c);
VOID DosDisplayOutput(COUNT c);
VOID FatGetDrvData(COUNT drive, COUNT FAR *spc, COUNT FAR *bps, COUNT FAR *nc, BYTE FAR **mdp);
WORD FcbParseFname(int wTestMode, BYTE FAR **lpFileName, fcb FAR *lpFcb);
BYTE FAR *ParseSkipWh(BYTE FAR *lpFileName);
BOOL TestCmnSeps(BYTE FAR *lpFileName);
BOOL TestFieldSeps(BYTE FAR *lpFileName);
BYTE FAR *GetNameField(BYTE FAR *lpFileName, BYTE FAR *lpDestField, COUNT nFieldSize, BOOL *pbWildCard);
BOOL FcbRead(xfcb FAR *lpXfcb, COUNT *nErrorCode);
BOOL FcbWrite(xfcb FAR *lpXfcb, COUNT *nErrorCode);
BOOL FcbGetFileSize(xfcb FAR *lpXfcb);
BOOL FcbSetRandom(xfcb FAR *lpXfcb);
BOOL FcbCalcRec(xfcb FAR *lpXfcb);
BOOL FcbRandomBlockRead(xfcb FAR *lpXfcb, COUNT nRecords, COUNT *nErrorCode);
BOOL FcbRandomBlockWrite(xfcb FAR *lpXfcb, COUNT nRecords, COUNT *nErrorCode);
BOOL FcbRandomRead(xfcb FAR *lpXfcb, COUNT *nErrorCode);
BOOL FcbRandomWrite(xfcb FAR *lpXfcb, COUNT *nErrorCode);
BOOL FcbCreate(xfcb FAR *lpXfcb);
void FcbNameInit(fcb FAR *lpFcb, BYTE *pszBuffer, COUNT *pCurDrive);
BOOL FcbOpen(xfcb FAR *lpXfcb);
BOOL FcbDelete(xfcb FAR *lpXfcb);
BOOL FcbRename(xfcb FAR *lpXfcb);
void MoveDirInfo(dmatch FAR *lpDmatch, struct dirent FAR *lpDir);
BOOL FcbClose(xfcb FAR *lpXfcb);
BOOL FcbFindFirst(xfcb FAR *lpXfcb);
BOOL FcbFindNext(xfcb FAR *lpXfcb);

/* initoem.c */
UWORD init_oem(void);

/* inthndlr.c */
VOID INRPT far int20_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags);
VOID INRPT far int21_handler(iregs UserRegs);
VOID far int21_entry(iregs UserRegs);
VOID int21_service(iregs far *r);
VOID INRPT FAR int22_handler(void);
VOID INRPT FAR int23_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags);
VOID INRPT FAR int24_handler(void);
VOID INRPT FAR int25_handler(void);
VOID INRPT FAR int26_handler(void);
VOID INRPT FAR int27_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags);
VOID INRPT FAR int28_handler(void);
VOID INRPT FAR int2f_handler(void);
VOID INRPT FAR empty_handler (void);

/* ioctl.c */
COUNT DosDevIOctl(iregs FAR *r, COUNT FAR *err);

/* main.c */
VOID main(void);
VOID reinit_k(void);
VOID init_device(struct dhdr FAR *dhp);
struct dhdr FAR *link_dhdr(struct dhdr FAR *lp, struct dhdr FAR *dhp);

/* memmgr.c */
VOID mcb_init (mcb FAR *mcbp, UWORD size);
seg far2para (VOID FAR *p);
seg long2para (LONG size);
VOID FAR *add_far (VOID FAR *fp, ULONG off);
VOID FAR *adjust_far (VOID FAR *fp);
COUNT DosMemAlloc (UWORD size, COUNT mode, seg FAR *para, UWORD FAR *asize);
seg DosMemLargest (seg FAR *size);
COUNT DosMemFree (UWORD para);
COUNT DosMemChange (UWORD para, UWORD size, UWORD *maxSize);
COUNT DosMemCheck (void);
COUNT FreeProcessMem (UWORD ps);
COUNT DosGetLargestBlock(UWORD FAR *block);
VOID show_chain (void);
VOID mcb_print (mcb FAR *mcbp);
VOID _fmemcpy(BYTE FAR *d, BYTE FAR *s, REG COUNT n);

/* misc.c */
VOID scopy(REG BYTE *s, REG BYTE *d);
VOID fscopy(REG BYTE FAR *s, REG BYTE FAR *d);
VOID fsncopy(BYTE FAR *s, BYTE FAR *d, REG COUNT n);
VOID bcopy(REG BYTE *s, REG BYTE *d, REG COUNT n);
VOID fbcopy(REG VOID FAR *s, REG VOID FAR *d, REG COUNT n);
BYTE *skipwh(BYTE *s);
BYTE *scan(BYTE *s, BYTE *d);
BYTE *scan_seperator(BYTE *s, BYTE *d);
BYTE *GetNumber(REG BYTE *pszString, REG COUNT *pnNum);
BOOL isnum(BYTE *pszString);

/* nls.c */
UWORD GetCtryInfo(UBYTE FAR *lpShrtCode, UWORD FAR *lpLongCode, BYTE FAR *lpTable);
UWORD SetCtryInfo(UBYTE FAR *lpShrtCode, UWORD FAR *lpLongCode, BYTE FAR *lpTable, UBYTE *nRetCode);
VOID INRPT FAR 
internalUpcase(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags);
BOOL GetGlblCodePage(UWORD FAR *ActvCodePage, UWORD FAR *SysCodePage);
BOOL SetGlblCodePage(UWORD FAR *ActvCodePage, UWORD FAR *SysCodePage);
BOOL ExtCtryInfo(UBYTE nOpCode, UWORD CodePageID, UWORD InfoSize, VOID FAR *Information);
char upMChar(UPMAP map, char ch);
VOID upMMem(UPMAP map, char FAR *str, unsigned len);
BYTE yesNo(char ch);
char upChar(char ch);
VOID upString(char FAR *str);
VOID upMem(char FAR *str, unsigned len);
char upFChar(char ch);
VOID upFString(char FAR *str);
VOID upFMem(char FAR *str, unsigned len);

/* prf.c */
VOID put_console(COUNT c);
WORD printf(CONST BYTE *fmt, ...);
WORD sprintf(BYTE *buff, CONST BYTE *fmt, ...);

/* strings.c */
COUNT strlen(REG BYTE *s);
COUNT fstrlen(REG BYTE FAR *s);
VOID strcpy(REG BYTE *d, REG BYTE *s);
VOID strncpy(REG BYTE *d, REG BYTE *s, COUNT l);
VOID strcat(REG BYTE *d, REG BYTE *s);
COUNT strcmp(REG BYTE *d, REG BYTE *s);
COUNT fstrcmp (REG BYTE FAR *d, REG BYTE FAR *s);
COUNT fstrncmp (REG BYTE FAR *d, REG BYTE FAR *s, COUNT l);
COUNT strncmp(REG BYTE *d, REG BYTE *s, COUNT l);
VOID fstrncpy (REG BYTE FAR *d, REG BYTE FAR FAR *s, COUNT l);
COUNT tolower(COUNT c);
COUNT toupper(COUNT c);

/* sysclk.c */
WORD clk_driver(rqptr rp);
COUNT BcdToByte(COUNT x);
COUNT BcdToWord(BYTE *x, UWORD *mon, UWORD *day, UWORD *yr);
COUNT ByteToBcd(COUNT x);
LONG WordToBcd(BYTE *x, UWORD *mon, UWORD *day, UWORD *yr);

/* syscon.c */
WORD con_driver(rqptr rp);
VOID break_handler(void);
VOID INRPT FAR int29_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags);

/* syspack.c */
VOID getdirent(BYTE FAR *vp, struct dirent FAR *dp);
VOID putdirent(struct dirent FAR *dp, BYTE FAR *vp);

/* systime.c */
VOID DosGetTime(BYTE FAR *hp, BYTE FAR FAR *mp, BYTE FAR FAR *sp, BYTE FAR FAR *hdp);
COUNT DosSetTime(BYTE FAR *hp, BYTE FAR FAR *mp, BYTE FAR FAR *sp, BYTE FAR FAR *hdp);
VOID DosGetDate(BYTE FAR *wdp, BYTE FAR FAR *mp, BYTE FAR FAR *mdp, COUNT FAR *yp);
COUNT DosSetDate(BYTE FAR *mp, BYTE FAR FAR *mdp, COUNT FAR *yp);

/* task.c */
COUNT ChildEnv (exec_blk FAR *exp, UWORD *pChildEnvSeg, char far *pathname);
VOID new_psp(psp FAR *p, int psize);
VOID return_user(void);
COUNT DosExec(COUNT mode, exec_blk FAR *ep, BYTE FAR *lp);
