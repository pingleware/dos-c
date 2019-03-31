/****************************************************************/
/*								*/
/*			    proto.h				*/
/*								*/
/*		     Global function prototypes			*/
/*								*/
/*			  August 9, 1991			*/
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

/* $Logfile:   C:/dos-c/src/command/proto.h_v  $ */

/*
 * $Log:   C:/dos-c/src/command/proto.h_v  $ 
   
      Rev 1.2   29 Aug 1996 13:07:06   patv
   Bug fixes for v0.91b
   
      Rev 1.1   01 Sep 1995 18:04:46   patv
   First GPL release.
   
      Rev 1.0   02 Jul 1995 10:02:32   patv
   Initial revision.
 */

#ifdef MAIN
static BYTE *Proto_hRcsId = "$Header:   C:/dos-c/src/command/proto.h_v   1.2   29 Aug 1996 13:07:06   patv  $";
#endif

#ifdef PROTO
/* home.c								*/
VOID FindHome(VOID);
BOOL GoHome(VOID);

/* scan.c								*/
VOID scopy(BYTE *, BYTE *);
VOID expand(BYTE *, BYTE *);
VOID expandspl(BYTE *, BYTE *, COUNT, BYTE *);
BYTE *scan(BYTE *, BYTE *);
BYTE *scanspl(BYTE *, BYTE *, COUNT);
BYTE *skipwh(BYTE *);
BYTE *scan_seperator(BYTE *, BYTE *);

/* cmd.c								*/
VOID do_command(COUNT);

/* cmd_err.c								*/
VOID error_message(enum error_mess);

/* dos_lib.c								*/
COUNT DosOpen(BYTE FAR *, COUNT);
COUNT DosCreat(BYTE FAR *, COUNT);
COUNT DosClose(COUNT);
COUNT DosRead(COUNT, BYTE FAR *, COUNT);
COUNT DosWrite(COUNT, BYTE FAR *, COUNT);
LONG DosSeek(COUNT, COUNT, LONG);
COUNT DosPwd(COUNT, BYTE FAR *);
COUNT DosSetDrive(COUNT);
COUNT DosGetDrive(VOID);
COUNT DosCd(BYTE FAR *);
VOID DosSetDta(BYTE FAR *);
COUNT DosFindFirst(COUNT, BYTE FAR *);
COUNT DosFindNext(VOID);
COUNT DosFree(COUNT, COUNT FAR *, COUNT FAR *, COUNT FAR *, COUNT FAR *);
VOID DosExit(COUNT);
COUNT DosGetftime(COUNT, date *, time *);
COUNT DosSetftime(COUNT, date *, time *);
COUNT DosDelete(BYTE FAR *);
COUNT DosRename(BYTE FAR *, BYTE FAR *);
COUNT DosMkdir(BYTE FAR *);
COUNT DosRmdir(BYTE FAR *);
COUNT DosExec(BYTE FAR *, exec_blk FAR *);
VOID DosGetTime(COUNT *, COUNT *, COUNT *, COUNT *);
COUNT DosSetTime(COUNT *, COUNT *, COUNT *, COUNT *);
VOID DosSetVerify(BOOL);
BOOL DosGetVerify(VOID);
BOOL DosGetBreak(VOID);
VOID DosSetBreak(BOOL);
VOID DosGetDate(COUNT *, COUNT *, COUNT *, COUNT *);
COUNT DosSetDate(COUNT *, COUNT *, COUNT *);
UWORD DosRtnValue(VOID);
BOOL DosMemAlloc(COUNT, BOOL FAR *);
BOOL DosMemFree(UWORD, BOOL FAR *);
VOID DosParseFilename(BYTE FAR *, fcb FAR *, BITS);


/* ../fs/prf.c								*/
COUNT printf(CONST BYTE *, ...);
COUNT sprintf(BYTE *, CONST BYTE *, ...);
VOID put_console(COUNT);
VOID scopy(BYTE *, BYTE *);
BYTE *skipwh(BYTE *);
COUNT strcmp(BYTE *, BYTE *);
BOOL dir(COUNT, BYTE *[]);

/* ../misc/strings.c							*/
COUNT strlen(BYTE *);
COUNT fstrlen(BYTE FAR *);
VOID strcpy(BYTE *, BYTE *);
VOID strcat(BYTE *, BYTE *);
COUNT strcmp(BYTE *, BYTE *);
COUNT strncmp(BYTE *, BYTE *);

/* ../misc/num.c							*/
COUNT tolower(COUNT);
COUNT toupper(COUNT);
COUNT atoi(BYTE *);

/* ../misc/dosopt.c							*/
BOOL iswild(BYTE *);
BOOL iseol(BYTE);
COUNT dosopt(BYTE *, BYTE FAR *, ...);
VOID expand(BYTE *, BYTE *);

/* env.c								*/
COUNT EnvSizeUp(VOID);
BYTE *EnvLookup(BYTE *);
BOOL EnvSetVar(BYTE *, BYTE *);
BOOL EnvClearVar(BYTE *);
BOOL EnvAlloc(COUNT);
BOOL EnvFree(BYTE FAR *);

#else
/* home.c								*/
VOID FindHome();
BOOL GoHome();

/* scan.c								*/
VOID scopy();
VOID expand();
VOID expandspl();
BYTE *scan();
BYTE *scanspl();
BYTE *skipwh();
BYTE *scan_seperator();

/* cmd.c								*/
VOID do_command();

/* cmd_err.c								*/
VOID error_message();

/* doslib.c								*/
COUNT DosOpen();
COUNT DosCreat();
COUNT DosClose();
COUNT DosRead();
COUNT DosWrite();
LONG DosSeek();
COUNT DosPwd();
COUNT DosSetDrive();
COUNT DosGetDrive();
COUNT DosCd();
VOID DosSetDta();
COUNT DosFindFirst();
COUNT DosFindNext();
COUNT DosFree();
VOID DosExit();
COUNT DosGetftime();
COUNT DosSetftime();
COUNT DosDelete();
COUNT DosRename();
COUNT DosMkdir();
COUNT DosRmdir();
COUNT DosExec();
VOID DosGetTime();
COUNT DosSetTime();
VOID DosSetVerify();
BOOL DosGetVerify();
BOOL DosGetBreak();
VOID DosSetBreak();
VOID DosGetDate();
COUNT DosSetDate();
UWORD DosRtnValue();
BOOL DosMemAlloc();
BOOL DosMemFree();
VOID DosParseFilename();


/* ../fs/prf.c								*/
COUNT printf ();
COUNT sprintf ();
VOID put_console();
VOID scopy();
BYTE *skipwh();
COUNT strcmp();
BOOL dir();

/* ../misc/strings.h							*/
COUNT strlen();
COUNT fstrlen();
VOID strcpy();
VOID strcat();
COUNT strcmp();
COUNT strncmp();

/* ../misc/num.c							*/
COUNT tolower();
COUNT toupper();
COUNT atoi();

/* ../misc/dosopt.c							*/
BOOL iswild();
BOOL iseol();
COUNT dosopt();
VOID expand();

/* env.c								*/
COUNT EnvSizeUp();
BYTE *EnvLookup();
BOOL EnvSetVar();
BOOL EnvClearVar();
BOOL EnvAlloc();
BOOL EnvFree();

#endif
