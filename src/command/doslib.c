/****************************************************************/
/*								*/
/*			     doslib.c				*/
/*								*/
/*		     Preliminary DOS SDK Functions 		*/
/*								*/
/*			  August 9, 1991			*/
/*								*/
/*		        Copyright (c) 1995			*/
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


/* $Logfile:   C:/dos-c/src/command/doslib.c_v  $ */

/*
 * $Log:   C:/dos-c/src/command/doslib.c_v  $ 
 * 
 *    Rev 1.2   29 Aug 1996 13:07:02   patv
 * Bug fixes for v0.91b
 * 
 *    Rev 1.1   01 Sep 1995 18:04:34   patv
 * First GPL release.
 * 
 *    Rev 1.0   02 Jul 1995 10:01:44   patv
 * Initial revision.
 */


#include "../../hdr/portab.h"
#include "globals.h"
#include "proto.h"

static BYTE *RcsId = "$Header:   C:/dos-c/src/command/doslib.c_v   1.2   29 Aug 1996 13:07:02   patv  $";

COUNT DosOpen(FileName, FileAccess)
COUNT FileAccess;
BYTE FAR *FileName;
{
	UWORD FileName_seg = FP_SEG(FileName);
	UWORD FileName_off = FP_OFF(FileName);
	COUNT Handle;

	asm {
	push	ds
	push	cx
	mov	cx,FileName_seg
	mov	dx,FileName_off
	mov	al,byte ptr FileAccess
	mov	ds,cx

	mov	ah,0x3d
	int	0x21

	pop	cx
	pop	ds

	mov	Handle,ax

	jnc	out
	neg	ax
	mov	Handle,ax

	}
out:	return Handle;
}


COUNT DosCreat(FileName, Attributes)
COUNT Attributes;
BYTE FAR *FileName;
{
	UWORD FileName_seg = FP_SEG(FileName);
	UWORD FileName_off = FP_OFF(FileName);
	COUNT Handle;

	asm {
	push	ds
	push	cx
	mov	ax,FileName_seg
	mov	dx,FileName_off
	mov	cx,Attributes
	mov	ds,ax

	mov	ah,0x3c
	int	0x21

	pop	cx
	pop	ds

	mov	Handle,ax

	jnc	out
	neg	ax
	mov	Handle,ax

	}
out:	return Handle;
}


COUNT DosClose(Handle)
COUNT Handle;
{
	COUNT status;

	asm {
	mov	bx,Handle

	mov	ah,0x3e
	int	0x21

	mov	status,ax
	jc	bad
	xor	ax,ax
	mov	status,ax
	}
bad:	return status == SUCCESS ? SUCCESS : -status;
}


COUNT DosRead (fd, buffer, count)
COUNT fd;
BYTE FAR *buffer;
COUNT count;
{
	COUNT nread = 0;
	UWORD buff_seg = FP_SEG(buffer);
	UWORD buff_off = FP_OFF(buffer);
	BOOL status = TRUE;

	asm {
	mov	bx,fd
	mov	cx,count
	mov	ax,buff_seg
	mov	dx,buff_off
	push	ds
	mov	ds,ax

	mov	ah,0x3f
	int	0x21

	pop	ds
	mov	nread,ax
	jnc	ok
	mov     dx,0
	mov	status,dx
	}
ok:	return status ? nread : -nread;
}

COUNT DosWrite (fd, buffer, count)
COUNT fd;
BYTE FAR *buffer;
COUNT count;
{
	COUNT nwrote = 0;
	UWORD buff_seg = FP_SEG(buffer);
	UWORD buff_off = FP_OFF(buffer);
	BOOL status = TRUE;

	asm {
	mov	bx,fd
	mov	cx,count
	mov	ax,buff_seg
	mov	dx,buff_off
	push	ds
	mov	ds,ax

	mov	ah,0x40
	int	0x21

	pop	ds
	mov	nwrote,ax
	jnc	ok
	mov     dx,0
	mov	status,dx
	}
ok:	return status ? nwrote : -nwrote;
}


LONG DosSeek(Handle, MoveMethod, FOffset)
COUNT Handle;
COUNT MoveMethod;
LONG FOffset;
{
	asm {
	mov	bx,Handle
	mov	dx,word ptr FOffset
	mov	cx,word ptr FOffset+2

	mov	al,byte ptr MoveMethod

	mov	ah,0x42
	int	0x21
	}
}


COUNT DosPwd(Drive, CurDir)
COUNT Drive;
BYTE FAR *CurDir;
{
	UWORD CurDir_seg = FP_SEG(CurDir);
	UWORD CurDir_off = FP_OFF(CurDir);
	COUNT status;

	asm {
	push	si
	mov	ax,CurDir_seg
	mov	si,CurDir_off
	mov	dl,byte ptr Drive
	push	ds
	mov	ds,ax

	mov	ah,0x47
	int	0x21

	pop	ds
	mov	status,ax
	jc	bad
	xor	ax,ax
	mov	status,ax
	}
bad:	return status == SUCCESS ? SUCCESS : -status;
}


COUNT DosSetDrive(DriveNumber)
COUNT DriveNumber;
{
	COUNT	LogicalDrives;

	asm {
	mov	dl,byte ptr DriveNumber

	mov	ah,0x0e
	int	0x21

	xor	ah,ah
	mov     LogicalDrives,ax
	}

	return LogicalDrives;
}


COUNT DosGetDrive()
{
	COUNT	DriveNumber;

	asm {
	mov	ah,0x19
	int	0x21

	xor	ah,ah
	mov     DriveNumber,ax
	}

	return DriveNumber;
}


COUNT DosCd(Dir)
BYTE FAR *Dir;
{
	UWORD Dir_seg = FP_SEG(Dir);
	UWORD Dir_off = FP_OFF(Dir);
	COUNT status;

	asm {
	push	ds
	mov	ax,Dir_seg
	mov	dx,Dir_off
	mov	ds,ax

	mov	ah,0x3b
	int	0x21

	pop	ds

	mov	status,ax
	jc	bad
	xor	ax,ax
	mov	status,ax
	}
bad:	return status;
}


VOID DosSetDta(DTA)
BYTE FAR *DTA;
{
	UWORD DTA_seg = FP_SEG(DTA);
	UWORD DTA_off = FP_OFF(DTA);

	asm {
	push	ds
	mov	ax,DTA_seg
	mov	dx,DTA_off
	mov	ds,ax

	mov	ah,0x1a
	int	0x21

	}
}



COUNT DosFindFirst(Attributes, FileName)
COUNT Attributes;
BYTE FAR *FileName;
{
	UWORD FileName_seg = FP_SEG(FileName);
	UWORD FileName_off = FP_OFF(FileName);
	COUNT status;

	asm {
	mov	cx,Attributes
	push	ds
	mov	ax,FileName_seg
	mov	dx,FileName_off
	mov	ds,ax

	mov	ah,0x4e
	int	0x21

	pop	ds

	mov	status,ax
	jc	bad
	xor	ax,ax
	mov	status,ax
	}
bad:	return status;
}


COUNT DosFindNext()
{
	COUNT status;

	asm {
	mov	ah,0x4f
	int	0x21

	mov	status,ax
	jc	bad
	xor	ax,ax
	mov	status,ax
	}
bad:	return status;
}

COUNT DosFree(Drive, Spc, Ac, Bpc, Tc)
COUNT Drive;
COUNT FAR *Spc, FAR *Ac, FAR *Bpc, FAR *Tc;
{
	COUNT SectorsPerCluster, AvailClusters, BytesPerSector, TotalClusters;
	COUNT status;

	asm {
	mov	dl,byte ptr Drive

	mov	ah,0x36
	int	0x21

	mov	status,ax
	cmp	ax,0xffff
	je	bad
	mov	SectorsPerCluster,ax
	mov	AvailClusters,bx
	mov	BytesPerSector,cx
	mov	TotalClusters,dx
	}

	*Spc =  SectorsPerCluster;
	*Ac =  AvailClusters;
	*Bpc = BytesPerSector;
	*Tc = TotalClusters;
bad:	return status;
}


VOID DosExit(ReturnValue)
COUNT ReturnValue;
{
	asm {
	mov	al,byte ptr ReturnValue

	mov	ah,0x4c
	int	0x21
	}
}

COUNT DosGetftime(fd, fdate, ftime)
COUNT fd;
date *fdate;
time *ftime;
{
	date FileDate;
	time FileTime;
	COUNT status;

	asm {
	mov	bx,fd

	mov	ax,0x5700
	int	0x21

	jc	out
	xor	ax,ax
	mov	status,ax
	mov	FileTime,cx
	mov	FileDate,dx
	}

out:	if(status == SUCCESS)
	{
		*fdate = FileDate;
		*ftime = FileTime;
		return SUCCESS;
	}
	else
		return  -status;
}


COUNT DosSetftime(fd, fdate, ftime)
COUNT fd;
date *fdate;
time *ftime;
{
	date FileDate = *fdate;
	time FileTime = *ftime;
	COUNT status;

	asm {
	mov	bx,fd
	mov	cx,FileTime
	mov	dx,FileDate

	mov	ax,0x5701
	int	0x21

	jc	out
	xor	ax,ax
	mov	status,ax
	}

out:	if(status == SUCCESS)
		return SUCCESS;
	else
		return  -status;
}


COUNT DosDelete(FileName)
BYTE FAR *FileName;
{
	UWORD FileName_seg = FP_SEG(FileName);
	UWORD FileName_off = FP_OFF(FileName);
	COUNT status;

	asm {
	push	ds
	mov	ax,FileName_seg
	mov	dx,FileName_off
	mov	ds,ax

	mov	ah,0x41
	int	0x21

	pop	ds
	mov	status,ax
	jc	bad
	xor	ax,ax
	mov	status,ax
	}
bad:	return status == SUCCESS ? SUCCESS : -status;
}


COUNT DosRename(OldName, NewName)
BYTE FAR *OldName;
BYTE FAR *NewName;
{
	UWORD OldName_seg = FP_SEG(OldName);
	UWORD OldName_off = FP_OFF(OldName);
	UWORD NewName_seg = FP_SEG(NewName);
	UWORD NewName_off = FP_OFF(NewName);
	COUNT status;

	asm {
	push	ds
	push	es
	push	di

	mov	ax,OldName_seg
	mov	dx,OldName_off
	mov	di,NewName_seg
	mov	es,di
	mov	di,NewName_off
	mov	ds,ax

	mov	ah,0x56
	int	0x21

	pop	di
	pop	es
	pop	ds
	mov	status,ax
	jc	bad
	xor	ax,ax
	mov	status,ax
	}
bad:	return status == SUCCESS ? SUCCESS : -status;
}


COUNT DosMkdir(Dir)
BYTE FAR *Dir;
{
	UWORD Dir_seg = FP_SEG(Dir);
	UWORD Dir_off = FP_OFF(Dir);
	COUNT status;

	asm {
	push	ds
	mov	ax,Dir_seg
	mov	dx,Dir_off
	mov	ds,ax

	mov	ah,0x39
	int	0x21

	pop	ds
	mov	status,ax
	jc	bad
	xor	ax,ax
	mov	status,ax
	}
bad:	return status == SUCCESS ? SUCCESS : -status;
}


COUNT DosRmdir(Dir)
BYTE FAR *Dir;
{
	UWORD Dir_seg = FP_SEG(Dir);
	UWORD Dir_off = FP_OFF(Dir);
	COUNT status;

	asm {
	push	ds
	mov	ax,Dir_seg
	mov	dx,Dir_off
	mov	ds,ax

	mov	ah,0x3a
	int	0x21

	pop	ds
	mov	status,ax
	jc	bad
	xor	ax,ax
	mov	status,ax
	}
bad:	return status == SUCCESS ? SUCCESS : -status;
}


COUNT DosExec(ProgName, ProgArgs)
BYTE FAR *ProgName;
exec_blk FAR *ProgArgs;
{
	UWORD ProgArgs_off = FP_OFF(ProgArgs);
	UWORD ProgArgs_seg = FP_SEG(ProgArgs);
	UWORD ProgName_off = FP_OFF(ProgName);
	UWORD ProgName_seg = FP_SEG(ProgName);
	COUNT status;

	asm {
	push	ds
	mov	dx,ProgName_off
	mov	ax,ProgName_seg
	mov	bx,ProgArgs_seg
	mov	es,bx
	mov	bx,ProgArgs_off
	mov	ds,ax

	mov	ax,0x4b00
	int	0x21

	pop	ds
	mov	status,ax
	jc	bad
	xor	ax,ax
	mov	status,ax
	}
bad:	return status == SUCCESS ? SUCCESS : -status;
}


UWORD DosRtnValue()
{
	UWORD status;

	asm {
	mov	ah,0x4d
	int	0x21

	mov	status,ax
	}
	return status;
}


VOID DosGetTime(hp, mp, sp, hdp)
COUNT *hp, *mp, *sp, *hdp;
{
	COUNT Hour = 0,
	Minutes = 0,
	Seconds = 0,
	Hundredths = 0;

	asm {
	mov	ah,0x2c
	int	0x21

	mov	byte ptr Hour,ch
	mov	byte ptr Minutes,cl
	mov	byte ptr Seconds,dh
	mov	byte ptr Hundredths,dl
	}

	*hp = Hour;
	*mp = Minutes;
	*sp = Seconds;
	*hdp = Hundredths;
}


COUNT DosSetTime(hp, mp, sp, hdp)
COUNT *hp, *mp, *sp, *hdp;
{
	COUNT Hour = *hp,
	Minutes = *mp,
	Seconds = *sp,
	Hundredths = *hdp;
	COUNT status;

	asm {
	mov	ch,byte ptr Hour
	mov	cl,byte ptr Minutes
	mov	dh,byte ptr Seconds
	mov	dl,byte ptr Hundredths

	mov	ah,0x2d
	int	0x21

	cmp	al,0
	jne	out
	xor	ax,ax
	mov	status,ax
	}
out:	return status == SUCCESS ? SUCCESS : DE_INVLDDATA;
}


VOID DosSetVerify(mode)
BOOL mode;
{
	asm {
	mov	al,byte ptr mode
	mov	ah,0x2E
	int	0x21
	}
}


BOOL DosGetVerify()
{
	asm {
	mov	ah,0x54
	int	0x21
	xor	ah,ah
	}
}


BOOL DosGetBreak()
{
	asm {
	mov	ax,0x3300
	int	0x21
	xor	ah,ah
	mov	al,dl
	}
}


VOID DosSetBreak(BreakFlag)
BOOL BreakFlag;
{
	BreakFlag = (BreakFlag != 0);
	asm {
	mov	dl,byte ptr BreakFlag
	mov	ax,0x3301
	int	0x21
	}
}


VOID DosGetDate(wdp, mp, mdp, yp)
COUNT *wdp, *mp, *mdp, *yp;
{
	COUNT WeekDay = 0,
	Month = 0,
	MonthDay = 0,
	Year = 0;

	asm {
	mov	ah,0x2a
	int	0x21

	mov	byte ptr WeekDay,al
	mov	Year,cx
	mov	byte ptr Month,dh
	mov	byte ptr MonthDay,dl
	}

	*wdp = WeekDay;
	*mp = Month;
	*mdp = MonthDay;
	*yp = Year;
}


COUNT DosSetDate(mp, mdp, yp)
COUNT *mp, *mdp, *yp;
{
	COUNT Month = *mp,
	MonthDay = *mdp,
	Year = *yp;
	COUNT status;

	asm {
	mov	cx,Year
	mov	dh,byte ptr Month
	mov	dl,byte ptr MonthDay

	mov	ah,0x2b
	int	0x21

	cmp	al,0
	jne	out
	xor	ax,ax
	mov	status,ax
	}
out:	return status == SUCCESS ? SUCCESS : DE_INVLDDATA;
}

BOOL DosCkKbReady()
{
	COUNT stat;

	asm {
	mov	ah,0x0b
	int	0x21
	xor	ah,ah
	mov	stat,ax
	}
	return (stat != 0);
}


COUNT DosRdKb()
{
	asm {
	mov	ah,0x08
	int	0x21
	xor	ah,ah
	}
}


UCOUNT DosAllocMem(MemSize, err)
UCOUNT MemSize;
BOOL FAR *err;
{
	UCOUNT SegmentMem;

	asm {
	mov	bx,word ptr MemSize

	mov	ah,0x48
	int	0x21

	jc	error_handler
	mov	SegmentMem,ax
	}
	*err = FALSE;
	return SegmentMem;

error_handler:
	asm {
	mov	SegmentMem,ax
	}
	*err = TRUE;
	return SegmentMem;
}




VOID DosFreeMem(SegmentMem, err)
UCOUNT SegmentMem;
BOOL FAR *err;
{
	asm {
	mov	ax,word ptr SegmentMem
	mov	es,ax

	mov	ah,0x49
	int	0x21

	jc	error_handler
	}
	*err = FALSE;
	return;

error_handler:
	*err = TRUE;
}


VOID DosParseFilename(pParseInput, pFileFCB, ParseControl)
BYTE FAR *pParseInput;
fcb FAR *pFileFCB;
BITS ParseControl;
{
	seg SegParseInput = FP_SEG(pParseInput);
	offset OffsetParseInput = FP_OFF(pParseInput);
	seg SegFileFCB = FP_SEG(pFileFCB);
	offset OffsetFileFCB = FP_OFF(pFileFCB);

	asm {
	push ds
	push si
	push di

	mov	si,SegParseInput
	mov	ds,si
	mov	si,OffsetParseInput
	mov	di,SegFileFCB
	mov	es,di
	mov	di,OffsetFileFCB
	mov	al,byte ptr ParseControl

	mov	ah,0x29
	int	0x21

	pop	di
	pop	si
	pop	ds
	}
}


BOOL DosDupHandle(OldHandle, pNewHandle, pErrorCode)
COUNT OldHandle;
COUNT FAR *pNewHandle;
COUNT FAR *pErrorCode;
{
	COUNT NewHandle, ReturnCode;

	asm {
	mov	bx,OldHandle

	mov	ah,0x45
	int	0x21

	jc	error_handler
	mov	NewHandle,ax
	}
	*pNewHandle = NewHandle;
	*pErrorCode = SUCCESS;
	return TRUE;

error_handler:
	asm {
	mov	ReturnCode,ax
	}
	*pErrorCode = -ReturnCode;
	return FALSE;
}

BOOL DosForceDupHandle(OpenHandle, DuplicateHandle, pErrorCode)
COUNT OpenHandle;
COUNT DuplicateHandle;
COUNT FAR *pErrorCode;
{
	COUNT ReturnCode;

	asm {
	mov	bx,OpenHandle
	mov	cx,DuplicateHandle

	mov	ah,0x46
	int	0x21

	jc	error_handler
	}
	*pErrorCode = SUCCESS;
	return TRUE;

error_handler:
	asm {
	mov	ReturnCode,ax
	}
	*pErrorCode = -ReturnCode;
	return FALSE;
}

