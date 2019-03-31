;
; File:
;			   int86.asm
; Description:
;	     generate requested 8086 interrupt -  self modifying
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
; $Logfile:   C:/dos-c/src/drivers/int86.asv  $
;
; $Header:   C:/dos-c/src/drivers/int86.asv   1.2   29 Aug 1996 13:07:08   patv  $
;
; $Log:   C:/dos-c/src/drivers/int86.asv  $
;
;   Rev 1.2   29 Aug 1996 13:07:08   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:38   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  7:59:22   patv
;Initial revision.
;

		page	60,132
		title	generate requested 8086 interrupt -  self modifying

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


_TEXT      	segment
		assume	CS: _TEXT
_int86		proc	near
		public	_int86

		push	bp
		mov	bp,sp
		push	si
		push	di
		push	ds
		push	bp
		mov	ax,[bp+4]
		mov	byte ptr cs:[word ptr selfx + 1],al
		mov	bx,[bp+6]
		mov	cx,[bx+4]
		mov	dx,[bx+6]
		mov	si,[bx+8]
		mov	di,[bx+0Ah]
		mov	bp,[bx+0Ch]
		mov	ax,[bx+2]
		push	ax
		mov	ax,[bx]
		push	ax
		mov	ax,[bx+10h]
		mov	es,ax
		push	word ptr [bx+12h]
		mov	ax,[bx+0Eh]
		mov	ds,ax
		popf				; Pop flags
		pop	ax
		pop	bx
selfx:		int	19h			; Bootstrap loader
		push	bp
		mov	bp,sp
		xchg	bx,[bp+2]
		mov	bp,bx
		mov	bx,ds
		xchg	bx,[bp-6]
		mov	ds,bx
		mov	bx,[bp+8]
		pushf				; Push flags
		pop	word ptr [bx+12h]
		mov	[bx],ax
		mov	[bx+4],cx
		mov	[bx+6],dx
		mov	[bx+8],si
		mov	[bx+0Ah],di
		mov	ax,es
		mov	[bx+10h],ax
		pop	word ptr [bx+0Ch]
		pop	word ptr [bx+2]
		pop	word ptr [bx+0Eh]
		pop	di
		pop	si
		pop	bp
		ret
  
_int86		endp
  
_TEXT		ends

		end
