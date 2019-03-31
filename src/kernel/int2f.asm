;
; File:
;			    int2f.asm
; Description:
;	 	  multiplex interrupt support code
;
;			Copyright (c) 1996
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
; $Logfile:   D:/dos-c/src/kernel/int2f.asv  $
;
; $Header:   D:/dos-c/src/kernel/int2f.asv   1.1   29 May 1996 21:03:46   patv  $
;
; $Log:   D:/dos-c/src/kernel/int2f.asv  $
; 
;    Rev 1.1   29 May 1996 21:03:46   patv
; bug fixes for v0.91a
; 
;    Rev 1.0   19 Feb 1996  3:34:38   patv
; Initial revision.
;

		page	60,132
		title	int 2fh handler

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

		public	_int2f_handler
_int2f_handler:
		sti				; Enable interrupts
		cmp	ah,11h			; Network interrupt?
		jne	Int2f3			; No, continue
Int2f1:
		or	al,al			; Installation check?
		jz	FarTabRetn		; yes, just return
Int2f2:

FarTabRetn:
		retf	2			; Return far
Int2f3:
		cmp	ah,10h			; SHARE.EXE interrupt?
		je	Int2f1			; yes, do installation check
		cmp	ah,14h			; NLSFUNC.EXE interrupt?
		je	Int2f1			; yes, do installation check
		iret				; Default, interrupt return

_TEXT		ends

		end
