;
; File:
;			  keyboard.asm
; Description:
;			keyboard primitives
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
; $Logfile:   C:/dos-c/src/drivers/keyboard.asv  $
;
; $Header:   C:/dos-c/src/drivers/keyboard.asv   1.2   29 Aug 1996 13:07:08   patv  $
;
; $Log:   C:/dos-c/src/drivers/keyboard.asv  $
;
;   Rev 1.2   29 Aug 1996 13:07:08   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:38   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  7:59:44   patv
;Initial revision.
;

		page	60,132
		title	keyboard primitives

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

_BSS	segment  word public 'BSS'
special	db	(?)
_BSS	ends


_TEXT		segment
		assume	CS: _TEXT

;
; COUNT kb_init(VOID)
;
;	Initializes the system keyboard.
;
  
_kb_init	proc	near
		public	_kb_init
		xor	al,al
		mov	DGROUP:special,al
		ret
_kb_init	endp


;
; COUNT kb_input(VOID)
;
;	Returns a character from the system keyboard.
;
  
_kb_input	proc	near
		public	_kb_input

		; Do a standard C entry
		push	bp
		mov	bp,sp

		; Test to see if last time we had a special character.
		; If so, just take it and run with it.
		mov	al,DGROUP:special
		or	al,al
		jnz	_kb_inp1

		; Use the BIOS for our input instead
		mov	ah,0
		int	16h			; Keyboard i/o  ah=function 00h
						;  get keybd char in al, ah=scan

		; Test if the BIOS returned a special character.
		; If it was, just save the top and send the null.
		or	al,al			; Bottom is null if special key
		jnz	_kb_inp1		; not, just return ascii part
		mov	DGROUP:special,ah	; save the special byte
		xor	ah,ah			; convert to a C int
		jmp	short _kb_inp2


_kb_inp1:	xor	ah,ah			; convert to a C int
		mov	DGROUP:special,ah	; clear the special byte

		; Standard C exit.
_kb_inp2:	pop	bp
		ret

_kb_input	endp
  
  
;==========================================================================
;			       SUBROUTINE
;==========================================================================
  
_kb_ndinput	proc	near
		public	_kb_ndinput

		push	bp
		mov	bp,sp
		mov	ah,1
		int	16h			; Keyboard i/o  ah=function 01h
						;  get status, if zf=0  al=char
		xor	ah,ah			; Zero register
		pop	bp
		ret

_kb_ndinput	endp
  
  
;==========================================================================
;			       SUBROUTINE
;==========================================================================
  
_kb_data	proc	near
		public	_kb_data

		push	bp
		mov	bp,sp
		mov	ah,1
		int	16h			; Keyboard i/o  ah=function 01h
						;  get status, if zf=0  al=char
		jz	kbd1			; Jump if zero
		xor	ah,ah			; Zero register
		jmp	short kbd2		; (0030)
		nop
kbd1:
		mov	ax,0FFFFh
kbd2:
		pop	bp
		ret

_kb_data	endp

_TEXT		ends

		end
