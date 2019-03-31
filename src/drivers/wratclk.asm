;
; File:
;			   wratclk.asm
; Description:
;		   WriteATClock - sysclock support
;
;			Copyright (c) 1995
;			Pasquale J. Villani
;			All Rights Reserved
;
; This file is part of DOS-C.
;
; DOS-C is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version
; 2, or (at your option) any later version.
;
; DOS-C is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
; the GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public
; License along with DOS-C; see the file COPYING.  If not,
; write to the Free Software Foundation, 675 Mass Ave,
; Cambridge, MA 02139, USA.
;
; $Logfile:   C:/dos-c/src/drivers/wratclk.asv  $
;
; $Header:   C:/dos-c/src/drivers/wratclk.asv   1.2   29 Aug 1996 13:07:12   patv  $
;
; $Log:   C:/dos-c/src/drivers/wratclk.asv  $
;
;   Rev 1.2   29 Aug 1996 13:07:12   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:42   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  8:01:18   patv
;Initial revision.
;

		page	60,132
		title	WriteATClock - sysclock support

IFDEF ??version
_TEXT		segment	byte public 'CODE'
DGROUP		group	_DATA,_BSS,_BSSEND		; small model
		assume	cs:_TEXT,ds:DGROUP,ss:DGROUP
_TEXT		ends

_DATA		segment word public 'DATA'
_DATA		ends

_BSS		segment word public 'BSS'
_BSS		ends

_BSSEND		segment byte public 'STACK'
_BSSEND		ends

ELSE
_TEXT		segment	byte public 'CODE'
_TEXT		ends

_DATA		segment word public 'DATA'
_DATA		ends

CONST		segment word public 'CONST'
CONST		ends

_BSS		segment word public 'BSS'
_BSS		ends

_BSSEND		segment byte public 'STACK'
_BSSEND		ends

DGROUP		group	CONST,_DATA,_BSS,_BSSEND	; small/tiny model
		assume	ds:DGROUP, ss:DGROUP
ENDIF

_TEXT		segment	byte public 'CODE'
		assume	cs:_TEXT


;
;	VOID WriteATClock(bcdDays, bcdHours, bcdMinutes, bcdSeconds)
;	BYTE *bcdDays;
;	BYTE bcdHours;
;	BYTE bcdMinutes;
;	BYTE bcdSeconds;
;
		PUBLIC	_WriteATClock
_WriteATClock	PROC NEAR
		push	bp
		mov	bp,sp
		sub	sp,4
;		LocalCopy = -4
;		bcdSeconds = 10
;		bcdMinutes = 8
;		bcdHours = 6
;		bcdDays = 4
		mov	bx,WORD PTR [bp+4]	;bcdDays
		mov	ax,WORD PTR [bx]
		mov	dx,WORD PTR [bx+2]
		mov	WORD PTR [bp-4],ax	;LocalCopy
		mov	WORD PTR [bp-2],dx
		mov	ch,BYTE PTR [bp+6]	;bcdHours
		mov	cl,BYTE PTR [bp+8]	;bcdMinutes
		mov	dh,BYTE PTR [bp+10]	;bcdSeconds
		mov	dl,0
		mov	ah,3
		int	26
		mov	cx,WORD PTR [bp-2]
		mov	dx,WORD PTR [bp-4]	;LocalCopy
		mov	ah,5
		int	26
		mov	sp,bp
		pop	bp
		ret	
		nop	

_WriteATClock	ENDP
_TEXT		ENDS
		END
