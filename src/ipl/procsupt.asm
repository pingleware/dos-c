;
; File:
;			  procsupt.asm
; Description:
;     Assembly support routines for process handling, etc.
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
; $Logfile:   C:/dos-c/src/ipl/procsupt.asv  $
;
; $Header:   C:/dos-c/src/ipl/procsupt.asv   1.2   29 Aug 1996 13:06:46   patv  $
;
; $Log:   C:/dos-c/src/ipl/procsupt.asv  $
;
;   Rev 1.2   29 Aug 1996 13:06:46   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 17:44:40   patv
;First GPL release.
;
;   Rev 1.0   02 Jul 1995  8:52:52   patv
;Initial revision.
;

		page	60,132
		title	Assembly support routines for process handling, etc.

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
; Borland C++ version
;

		extrn	_api_sp:word		; api stacks - for context
		extrn	_api_ss:word		; switching
		extrn	_usr_sp:word		; user stacks
		extrn	_usr_ss:word

		extrn	_kstackp:near		; kernel stack
		extrn	_ustackp:near		; new task stack

		extrn	_break_flg:near		; break detected flag

_TEXT      	segment
		assume	cs: _TEXT

;
;	Special call for switching processes
;
;	void interrupt far exec_user(irp)
;	iregs far *irp;
;
;	Borland C++ v2.0 specific
;
;	+---------------+
;	|    irp hi	|	26
;	+---------------+
;	|    irp low	|	24
;	+---------------+
;	|     flags	|	22
;	+---------------+
;	|	cs	|	20
;	+---------------+
;	|	ip	|	18
;	+---------------+
;	|	ax	|	16
;	+---------------+
;	|	bx	|	14
;	+---------------+
;	|	cx	|	12
;	+---------------+
;	|	dx	|	10
;	+---------------+
;	|	es	|	8
;	+---------------+
;	|	ds	|	6
;	+---------------+
;	|	si	|	4
;	+---------------+
;	|	di	|	2
;	+---------------+
;	|	bp	|	0
;	+---------------+
;
		public	_exec_user
_exec_user	proc far

		push	ax
		push	bx
		push	cx
		push	dx
		push	es
		push	ds
		push	si
		push	di
		push	bp
		mov	bp,sp
;
;
;
		mov	ax,WORD PTR [bp+24]	; irp (user ss:sp)
		mov	dx,WORD PTR [bp+26]
		cld
		cli
		mov	sp,ax			; set-up user stack
		mov	ss,dx
		sti
;
		pop	bp
		pop	di
		pop	si
		pop	ds
		pop	es
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		iret	

_exec_user		endp





;
; interrupt enable and disable routines
;
		public	_enable
_enable		proc near
		sti
		ret
_enable		endp

		public	_disable
_disable	proc near
		cli
		ret
_disable	endp

_TEXT		ends

		end

