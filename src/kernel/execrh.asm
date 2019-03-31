;
; File:
;			   execrh.asm
; Description:
;	      request handler for calling device drivers
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
; $Logfile:   D:/dos-c/src/kernel/execrh.asv  $
;
; $Header:   D:/dos-c/src/kernel/execrh.asv   1.2   29 May 1996 21:03:30   patv  $
;
; $Log:   D:/dos-c/src/kernel/execrh.asv  $
;
;   Rev 1.2   29 May 1996 21:03:30   patv
;bug fixes for v0.91a
;
;   Rev 1.1   01 Sep 1995 17:54:22   patv
;First GPL release.
;
;   Rev 1.0   02 Jul 1995  9:05:34   patv
;Initial revision.
;

		page	60,132
		title	request handler for calling device drivers

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
		; _execrh
		;	Execute Device Request
		;
		; execrh(rhp, dhp)
		; request far *rhp;
		; struct dhdr far *dhp;
		;
_execrh		proc	near
		public _execrh

		push	bp			; perform c entry
		mov	bp,sp
		sub	sp,4
		push	si
		push	ds
		lds	si,dword ptr [bp+8]	; ds:si = device header
		les	bx,dword ptr [bp+4]	; es:bx = request header
		mov	ax,[si+6]		; get strategy dword address
		mov	word ptr [bp-4],ax	; driver offset
		mov	word ptr [bp-2],ds	; driver cs = ds from pointer
		call	dword ptr ss:[bp-4]	;call device strategy
		mov	ax,[si+8]		;get interrupt dword address
		mov	word ptr [bp-4],ax
		call	dword ptr ss:[bp-4]	;call device interrupt
		pop	ds
		pop	si
		mov	sp,bp
		pop	bp
		ret

_execrh		endp

_TEXT		ends
		end
