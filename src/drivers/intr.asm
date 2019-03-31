;
; File:
;			   intr.asm
; Description:
;		save 8086 registers during interrupts
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
; $Logfile:   C:/dos-c/src/drivers/intr.asv  $
;
; $Header:   C:/dos-c/src/drivers/intr.asv   1.2   29 Aug 1996 13:07:14   patv  $
;
; $Log:   C:/dos-c/src/drivers/intr.asv  $
;
;   Rev 1.2   29 Aug 1996 13:07:14   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:36   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  7:58:12   patv
;Initial revision.
;

		page	60,132
		title	save 8086 registers during interrupts

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

;
; i_save stack frame
;
; entry:
;
;			flags
;			cs
;			ip
;	sp ->		asm call return address
;
; exit:
;			flags
;			cs
;			ip
;			ax
;			bp
;			di
;			si
;			bx
;			cx
;			dx
;			ds
;			es
;	sp ->		asm call return address
;
;	All registers remained unchanged.
;
;

_TEXT      	segment
		assume	CS: _TEXT
i_save		proc	near
		public	i_save

		push	bp		; start stacking
		mov	bp,sp		; and save pointer to the entry stack
		push	di
		push	si
		push	bx
		push	cx
		push	dx
		push	ds
		push	es
		xchg	ax,[bp+2]	; swap ax and return address
		push	ax		; and put return on stack for exit
		mov	ax,[bp+2]	; get ax back
		mov	bp,[bp]		; get bp back
		ret

i_save		endp

i_exit		proc	near
		public	i_exit

		pop	es		; just unstack from an isave
		pop	ds
		pop	dx
		pop	cx
		pop	bx
		pop	si
		pop	di
		pop	bp
		pop	ax
		iret				; Interrupt return
  
i_exit		endp

_TEXT		ends

		end
