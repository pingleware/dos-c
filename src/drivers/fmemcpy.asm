;
; File:
;			  fmemcpy.asm
; Description:
;		memory copy primitive - no mmu support
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
; $Logfile:   C:/dos-c/src/drivers/fmemcpy.asv  $
;
; $Header:   C:/dos-c/src/drivers/fmemcpy.asv   1.2   29 Aug 1996 13:07:08   patv  $
;
; $Log:   C:/dos-c/src/drivers/fmemcpy.asv  $
;
;   Rev 1.2   29 Aug 1996 13:07:08   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:38   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  7:58:52   patv
;Initial revision.
;

		page	60,132
		title	memory copy primitive - no mmu support

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
_fmemcpy	proc	near
		public	_fmemcpy

		push	bp
		mov	bp,sp
		push	si
		push	di
		push	ds
		push	es
		cld				; Clear direction
		lds	si,dword ptr [bp+8]	; Load 32 bit ptr
		mov	ax,ds
		xor	dx,dx			; Zero register
		mov	cx,4
  
fmemcpy1:
		shl	ax,1			; Shift w/zeros fill
		rcl	dx,1			; Rotate thru carry
		loop	fmemcpy1		; Loop if cx > 0
  
		add	ax,si
		adc	dx,0
		mov	si,ax
		and	si,0Fh
		mov	cx,4
  
fmemcpy2:
		clc				; Clear carry flag
		rcr	dx,1			; Rotate thru carry
		rcr	ax,1			; Rotate thru carry
		loop	fmemcpy2		; Loop if cx > 0
  
		mov	ds,ax
		les	di,dword ptr [bp+4]	; Load 32 bit ptr
		mov	ax,es
		xor	dx,dx			; Zero register
		mov	cx,4
  
fmemcpy3:
		shl	ax,1			; Shift w/zeros fill
		rcl	dx,1			; Rotate thru carry
		loop	fmemcpy3		; Loop if cx > 0
  
		add	ax,di
		adc	dx,0
		mov	di,ax
		and	di,0Fh
		mov	cx,4
  
fmemcpy4:
		clc				; Clear carry flag
		rcr	dx,1			; Rotate thru carry
		rcr	ax,1			; Rotate thru carry
		loop	fmemcpy4		; Loop if cx > 0
  
		mov	es,ax
		mov	ax,0FFF0h
		and	ax,[bp+0Ch]
		cmp	ax,0FFF0h
		jne	fmemcpy5			; Jump if not equal
		push	si
		push	di
		mov	cx,ax
		rep	movsb			; Rep while cx>0 Mov [si] to es:[di]
		mov	ax,es
		add	ax,0FFFh
		mov	es,ax
		mov	ax,ds
		add	ax,0FFFh
		mov	ds,ax
		mov	cx,0Fh
		and	cx,[bp+0Ch]
		pop	di
		pop	si
		jmp	short fmemcpy6		; (0081)
		nop
fmemcpy5:
		mov	cx,[bp+0Ch]
fmemcpy6:
		rep	movsb			; Rep while cx>0 Mov [si] to es:[di]
		les	di,dword ptr [bp+4]	; Load 32 bit ptr
		mov	ax,di
		mov	dx,es
		pop	es
		pop	ds
		pop	di
		pop	si
		pop	bp
		ret
  
_fmemcpy	endp
  
_TEXT		ends

		end
