;
; File:
;			   getvec.asm
; Description:
;		get an interrupt vector - simple version
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
; $Logfile:   C:/dos-c/src/drivers/getvec.asv  $
;
; $Header:   C:/dos-c/src/drivers/getvec.asv   1.2   29 Aug 1996 13:07:10   patv  $
;
; $Log:   C:/dos-c/src/drivers/getvec.asv  $
;
;   Rev 1.2   29 Aug 1996 13:07:10   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:40   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  8:00:36   patv
;Initial revision.
;

		page	60,132
		title	get an interrupt vector - simple version

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
_getvec		proc	near
		public	_getvec

		push	bp
		mov	bp,sp
		mov	ax,[bp+4]
		call	getvec
		pop	bp
		ret
  
_getvec		endp
  
;
; assembler version - ax = vector number
;	returns vector in dx:ax
;
  
getvec		proc	near
		public	getvec

		shl	ax,1			; Multiply by 4
		shl	ax,1
		xor	dx,dx			; and set segment to 0
		mov	es,dx
		mov	bx,ax
		pushf				; Push flags
		cli				; Disable interrupts
		mov	ax,es:[bx]
		mov	dx,es:[bx+2]
		popf				; Pop flags
		ret
getvec		endp

_TEXT		ends

		end
