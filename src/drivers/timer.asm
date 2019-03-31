;
; File:
;			   timer.asm
; Description:
;	      Set a single timer and check when expired
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
; $Logfile:   C:/dos-c/src/drivers/timer.asv  $
;
; $Header:   C:/dos-c/src/drivers/timer.asv   1.2   29 Aug 1996 13:07:12   patv  $
;
; $Log:   C:/dos-c/src/drivers/timer.asv  $
;
;   Rev 1.2   29 Aug 1996 13:07:12   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:42   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  8:01:04   patv
;Initial revision.
;

		page	60,132
		title	Set a single timer and check when expired

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

_TEXT	segment byte public 'CODE'
;	
;	void tmark()
;	
	assume	cs:_TEXT
_tmark	proc	near
	push	bp
	mov	bp,sp
	xor	ah,ah
	int	01aH			; get current time in ticks
	xor	ah,ah
	mov	word ptr LastTime,dx	; and store it
	mov	word ptr LastTime+2,cx
	pop	bp
	ret	
_tmark	endp


;	
;	int tdelay(Ticks)
;	
	assume	cs:_TEXT
_tdelay	proc	near
	push	bp
	mov	bp,sp
	sub	sp,4
	xor	ah,ah
	int	01aH			; get current time in ticks
	xor	ah,ah
	mov	word ptr [bp-4],dx	; and save it to a local variable
	mov	word ptr [bp-4]+2,cx	; "Ticks"
;
; Do a c equivalent of:
;
;		return Now >= (LastTime + Ticks);
;	
	mov	ax,word ptr DGROUP:LastTime+2
	mov	dx,word ptr DGROUP:LastTime
	add	dx,word ptr [bp+4]
	adc	ax,word ptr [bp+6]
	cmp	ax,word ptr [bp-2]
	ja	short tdel_1
	jne	short tdel_2
	cmp	dx,word ptr [bp-4]
	ja	short tdel_1
tdel_2:
	mov	ax,1			; True return
	jmp	short tdel_3
tdel_1:
	xor	ax,ax			; False return
tdel_3:
	mov	sp,bp
	pop	bp
	ret	
_tdelay	endp


;	
;	void twait(Ticks)
;	
	assume	cs:_TEXT
_twait	proc	near
	push	bp
	mov	bp,sp
	sub	sp,4
	call	near ptr _tmark			; mark a start
;
;	c equivalent
;		do
;			GetNowTime(&Now);
;		while((LastTime + Ticks) < Now);
twait_1:
	xor	ah,ah
	int	01aH
	xor	ah,ah				; do GetNowTime
	mov	word ptr [bp-4],dx		; and save it to "Now"
	mov	word ptr [bp-4]+2,cx
;
;	do comparison
;
	mov	ax,word ptr DGROUP:LastTime+2
	mov	dx,word ptr DGROUP:LastTime
	add	dx,word ptr [bp+4]
	adc	ax,word ptr [bp+6]
	cmp	ax,word ptr [bp-2]
	jb	short twait_1
	jne	short twait_2
	cmp	dx,word ptr [bp-4]
	jb	short twait_1
twait_2:
	mov	sp,bp
	pop	bp
	ret	
_twait	endp
_TEXT	ends

_BSS	segment word public 'BSS'
LastTime	label	word
	db	4 dup (?)
_BSS	ends

_TEXT	segment byte public 'CODE'
_TEXT	ends
	public	_twait
	public	_tdelay
	public	_tmark
	end
