;
; File:
;			  console.asm
; Description:
;		console device primitives for PC Clones
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
; $Logfile:   C:/dos-c/src/drivers/console.asv  $
;
; $Header:   C:/dos-c/src/drivers/console.asv   1.2   29 Aug 1996 13:07:06   patv  $
;
; $Log:   C:/dos-c/src/drivers/console.asv  $
;
;   Rev 1.2   29 Aug 1996 13:07:06   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:36   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  7:58:24   patv
;Initial revision.
;

		page	60,132
		title	console device primitives for PC Clones

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

; $Logfile:   C:/dos-c/src/drivers/console.asv  $
;
; $Header:   C:/dos-c/src/drivers/console.asv   1.2   29 Aug 1996 13:07:06   patv  $
;
; $Log$
;
_TEXT      segment
	assume	CS: _TEXT
;
; con - output to console
;	near version
;
_con	proc	near
	public	_con
	push	bp
	mov	bp,sp
	push	bx
	mov	ax,word ptr [bp+4]	; get *s
	mov	ah,0eh			; and output the byte
	mov	bx,7
	int	10h
	pop	bx
	pop	bp
	ret
_con	endp


_TEXT	ends

	end
