/****************************************************************/
/*								*/
/*			     doslib.c				*/
/*								*/
/*		       DOS Emulation Library			*/
/*								*/
/*			 November 6, 1991			*/
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


/* $Logfile*/
static char *doslibRcsId = "$Header:   C:/dos-c/src/misc/doslib.c_v   1.2   29 Aug 1996 13:07:36   patv  $";

/*
 * $Log:   C:/dos-c/src/misc/doslib.c_v  $
 *	
 *	   Rev 1.2   29 Aug 1996 13:07:36   patv
 *	Bug fixes for v0.91b
 *	
 *	   Rev 1.1   01 Sep 1995 18:11:00   patv
 *	First GPL release.
 *	
 *	   Rev 1.0   02 Jul 1995 11:04:44   patv
 *	Initial revision.
 */


#include <dos.h>
#include "../hdr/device.h"
#include "../hdr/time.h"
#include "../hdr/date.h"
#include "../hdr/fcb.h"
#include "../hdr/process.h"
#include "doslib.h"


/*!Function int DosGetDirectory();	*/
int DosGetDirectoryPath(drive, buffer)
int drive;
char far *buffer;
{
	union REGS r;
	struct SREGS s;

	r.h.ah = 0x47;
	r.h.dl = drive;
	r.x.si = FP_OFF(buffer);
	s.ds = FP_SEG(buffer);
	int86x(0x21, &r, &r, &s);
	if(r.x.cflag)
		return -r.x.ax;
	else
		return 0;
}


/*!Function int DosOpen();			*/
int DosOpen(path, flag)
char far *path;
int flag;
{
	union REGS r;
	struct SREGS s;

	r.h.ah = 0x3d;
	r.h.al = flag;
	r.x.dx = FP_OFF(path);
	s.ds = FP_SEG(path);
	int86x(0x21, &r, &r, &s);
	if(r.x.cflag)
		return -r.x.ax;
	else
		return r.x.ax;
}



/*!Function int DosRead();			*/
int DosRead(hndl, buf, cnt)
int hndl;
char far *buf;
int cnt;
{
	union REGS r;
	struct SREGS s;

	r.h.ah = 0x3f;
	r.x.bx = hndl;
	r.x.cx = cnt;
	r.x.dx = FP_OFF(buf);
	s.ds = FP_SEG(buf);
	int86x(0x21, &r, &r, &s);
	if(r.x.cflag)
		return -r.x.ax;
	else
		return r.x.ax;
}


/*!Function int DosClose();			*/
int DosClose(hndl)
int hndl;
{
	union REGS r;
	struct SREGS s;

	r.h.ah = 0x3d;
	r.x.bx = hndl;
	int86x(0x21, &r, &r, &s);
	if(r.x.cflag)
		return -r.x.ax;
	else
		return 0;
}


/*!Function void DosProcessTerminate();		*/
void DosProcessTerminate(code)
int code;
{
	exit(code);
}


/*!Function int DosSetDTA();			*/
int DosSetDTA(p)
char far *p;
{
	union REGS r;
	struct SREGS s;

	r.h.ah = 0x1a;
	r.x.dx = FP_OFF(p);
	s.ds = FP_SEG(p);
	int86x(0x21, &r, &r, &s);
	if(r.x.cflag)
		return -r.x.ax;
	else
		return 0;
}


/*Function int DosFindFirst();				*/
int DosFindFirst(attr, name)
int attr;
char far *name;
{
	union REGS r;
	struct SREGS s;

	r.h.ah = 0x4e;
	r.x.cx = attr;
	r.x.dx = FP_OFF(name);
	s.ds = FP_SEG(name);
	int86x(0x21, &r, &r, &s);
	if(r.x.cflag)
		return -r.x.ax;
	else
		return 0;
}


/*Function int DosFindNext();				*/
int DosFindNext()
{
	union REGS r;

	r.h.ah = 0x4f;
	int86(0x21, &r, &r);
	if(r.x.cflag)
		return -r.x.ax;
	else
		return 0;
}


/*Function int DosDirPath();				*/
int DosDirPath(drive, pathp)
int drive;
char far *pathp;
{
	union REGS r;
	struct SREGS s;

	r.h.ah = 0x47;
	s.ds = FP_SEG(pathp);
	r.x.si = FP_OFF(pathp);
	r.h.dl = drive;
	int86x(0x21, &r, &r, &s);
	if(r.x.cflag)
		return -r.x.ax;
	else
		return 0;
}


/*Function int DosSetDefaultDrive();				*/
int DosSetDefaultDrive(drive)
int drive;
{
	union REGS r;

	r.h.ah = 0x0e;
	r.h.dl = drive;
	int86(0x21, &r, &r);
	return r.h.al;
}


/*Function int DosGetCurrentDrive();				*/
int DosGetCurrentDrive()
{
	union REGS r;

	r.h.ah = 0x19;
	int86(0x21, &r, &r);
	return r.h.al;
}


/*Function int DosDiskFreeSpace();				*/
int DosDiskFreeSpace(drive, av_clp, bpsp, max_clusp)
int drive, *av_clp, *bpsp, *max_clusp;
{
	union REGS r;

	r.h.ah = 0x36;
	r.h.dl = drive;
	int86(0x21, &r, &r);
	*av_clp = r.x.bx;
	*bpsp = r.x.cx;
	*max_clusp = r.x.dx;
	return r.x.ax;
}


/*Function int DosFATParseName();				*/
int DosFATParseName(control, string, fcb)
int control;
char far **string;
fcb far *fcb;
{
	union REGS r;
	struct SREGS s;

	r.h.ah = 0x29;
	r.h.al = control;
	s.ds = FP_SEG(*string);
	r.x.si = FP_OFF(*string);
	s.es = FP_SEG(fcb);
	r.x.di = FP_OFF(fcb);
	int86(0x21, &r, &r);
	*string = MK_FP(s.ds, r.x.si);
	return r.x.ax;
}

/*Function int DosExec();				*/
int DosExec(fn, ep, lp)
int fn;
exec_blk far *ep;
char far *lp;
{
	union REGS r;
	struct SREGS s;

	r.h.ah = 0x4b;
	r.h.al = fn;
	r.x.bx = FP_OFF(ep);
	s.es = FP_SEG(ep);
	r.x.dx = FP_OFF(lp);
	s.ds = FP_SEG(lp);
	int86x(0x21, &r, &r, &s);
	if(r.x.cflag)
		return -r.x.ax;
	else
		return 0;
}

