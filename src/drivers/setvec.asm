;
; File:
;			   setvec.asm
; Description:
;		set an interrupt vector - simple version
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
; $Logfile:   C:/dos-c/src/drivers/setvec.asv  $
;
; $Header:   C:/dos-c/src/drivers/setvec.asv   1.2   29 Aug 1996 13:07:12   patv  $
;
; $Log:   C:/dos-c/src/drivers/setvec.asv  $
;
;   Rev 1.2   29 Aug 1996 13:07:12   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:36   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  7:57:44   patv
;Initial revision.
;

		page	60,132
		title	set an interrupt vector - simple version

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


_TEXT		segment
		assume	CS: _TEXT
_setvec		proc	near
		public	_setvec

		push	bp
		mov	bp,sp
		mov	bx,[bp+4]
		mov	ax,[bp+6]
		mov	dx,[bp+8]
		call	setvec
		pop	bp
		ret
  
_setvec		endp
  
;
; assembler version - bx = int #, dx:ax = vector
;		returns old vector in dx:ax
;
  
setvec		proc	near
		public	setvec

		shl	bx,1			; Multiply by 4
		shl	bx,1
		xor	cx,cx			; set segment to 0
		mov	es,cx
		pushf				; Push flags
		cli				; Disable interrupts
		xchg	ax,es:[bx]
		xchg	dx,es:[bx+2]
		popf				; Pop flags
		ret
setvec		endp

_TEXT		ends

		end
