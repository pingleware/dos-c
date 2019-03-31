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
; $Logfile:   D:/dos-c/src/kernel/procsupt.asv  $
;
; $Header:   D:/dos-c/src/kernel/procsupt.asv   1.2   29 May 1996 21:03:36   patv  $
;
; $Log:   D:/dos-c/src/kernel/procsupt.asv  $
;
;   Rev 1.2   29 May 1996 21:03:36   patv
;bug fixes for v0.91a
;
;   Rev 1.1   01 Sep 1995 17:54:24   patv
;First GPL release.
;
;   Rev 1.0   02 Jul 1995  9:05:58   patv
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


IFDEF ??version
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
		extrn	_int21_entry:far	; far call system services

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
		mov	ax,DGROUP
		mov	ds,ax
		push	bp
		mov	bp,sp
		cld
		cli	
;
;
;
		mov	ax,WORD PTR [bp+24]	; irp (user ss:sp)
		mov	dx,WORD PTR [bp+26]
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
;	Special call for switching processes during break handling
;
;	void interrupt far handle_break()
;
;
;	+---------------+
;	|     flags	|	24
;	+---------------+
;	|	cs	|	22
;	+---------------+
;	|	ip	|	20
;	+---------------+
;	|	ax	|	18
;	+---------------+
;	|	cx	|	16
;	+---------------+
;	|	dx	|	14
;	+---------------+
;	|	bx	|	12
;	+---------------+
;	|	sp	|	10
;	+---------------+
;	|	bp	|	8
;	+---------------+
;	|	si	|	6
;	+---------------+
;	|	di	|	4
;	+---------------+
;	|	ds	|	2
;	+---------------+
;	|	es	|	0	<--- bp & sp after mov bp,sp
;	+---------------+
;
		public	_handle_break
_handle_break	proc far

		push	ax
		push	bx
		push	cx
		push	dx
		push	es
		push	ds
		push	si
		push	di
		mov	ax,DGROUP
		mov	ds,ax
		push	bp
		mov	bp,sp
		assume DS: DGROUP
		cld

		; handler body - start out by restoring stack
		pushf
		cli	

		; save background stack
		mov	word ptr DGROUP:_api_ss,ss
		mov	word ptr DGROUP:_api_sp,sp

		; restore foreground stack here
		mov	ss,word ptr DGROUP:_usr_ss
		mov	sp,word ptr DGROUP:_usr_sp

		; get all the user registers back
		pop	bp
		pop	di
		pop	si
		pop	ds
		pop	es
		pop	dx
		pop	cx
		pop	bx
		pop	ax

		; do the int 23 handler and see if it returns
		int	23h

		; we're back, must have been users handler
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
		mov	ax,DGROUP
		mov	ds,ax
		assume DS: DGROUP

		; test for far return or iret
		cmp	sp,_usr_sp
		jz	hbrk1		; it was far ret

		; restart int 21 from the top
hbrk1:		pop	bp
		pop	di
		pop	si
		pop	ds
		pop	es
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		call	_int21_entry
		iret

hbrk2:		popf			; clear the flag from the stack
		jnc	hbrk1		; user wants to restart
		pop	bp
		pop	di
		pop	si
		pop	ds
		pop	es
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		mov	ax,4c00h	; exit
		mov	byte ptr _break_flg,0ffh ; set break detected flag
		call	_int21_entry
		iret

		
_handle_break	endp

ELSE
;
; Microsoft C version
;

		extrn	_api_sp:word		; api stacks - for context
		extrn	_api_ss:word		; switching
		extrn	_usr_sp:word		; user stacks
		extrn	_usr_ss:word

		extrn	_kstackp:near		; kernel stack
		extrn	_ustackp:near		; new task stack

		extrn	_break_flg:near		; break detected flag
		extrn	_int21_entry:far	; far call system services

_TEXT      	segment
		assume	cs: _TEXT

;
;	Special call for switching processes
;
;	void interrupt far exec_user(irp)
;	iregs far *irp;
;
;
;	+---------------+
;	|    irp hi	|	28
;	+---------------+
;	|    irp low	|	26
;	+---------------+
;	|     flags	|	24
;	+---------------+
;	|	cs	|	22
;	+---------------+
;	|	ip	|	20
;	+---------------+
;	|	ax	|	18
;	+---------------+
;	|	cx	|	16
;	+---------------+
;	|	dx	|	14
;	+---------------+
;	|	bx	|	12
;	+---------------+
;	|	sp	|	10
;	+---------------+
;	|	bp	|	8
;	+---------------+
;	|	si	|	6
;	+---------------+
;	|	di	|	4
;	+---------------+
;	|	ds	|	2
;	+---------------+
;	|	es	|	0	<--- bp & sp after mov bp,sp
;	+---------------+
;
		public	_exec_user
_exec_user	proc far

		push	ax
		push	cx
		push	dx
		push	bx
		push	sp
		push	bp
		push	si
		push	di
		push	ds
		push	es
		mov	bp,sp
		mov	ax,DGROUP
		mov	ds,ax
		assume DS: DGROUP
		cld
		cli	
;
;
;
		mov	ax,sp			; get kernel stack stack
		mov	dx,ss
		mov	word ptr _kstackp,ax	; save kernel stack stack
		mov	word ptr _kstackp+2,dx
		mov	ax,WORD PTR [bp+26]	; irp (user ss:sp)
		mov	dx,WORD PTR [bp+28]
		mov	sp,ax			; set-up user stack
		mov	ss,dx
;
		pop	es
		pop	ds
		pop	di
		pop	si
		pop	bp
		pop	bx
		pop	bx
		pop	dx
		pop	cx
		pop	ax
		iret	

_exec_user		endp


;
;	Special call for switching processes during break handling
;
;	void interrupt far handle_break()
;
;
;	+---------------+
;	|     flags	|	24
;	+---------------+
;	|	cs	|	22
;	+---------------+
;	|	ip	|	20
;	+---------------+
;	|	ax	|	18
;	+---------------+
;	|	cx	|	16
;	+---------------+
;	|	dx	|	14
;	+---------------+
;	|	bx	|	12
;	+---------------+
;	|	sp	|	10
;	+---------------+
;	|	bp	|	8
;	+---------------+
;	|	si	|	6
;	+---------------+
;	|	di	|	4
;	+---------------+
;	|	ds	|	2
;	+---------------+
;	|	es	|	0	<--- bp & sp after mov bp,sp
;	+---------------+
;
		public	_handle_break
_handle_break	proc far

		push	ax
		push	cx
		push	dx
		push	bx
		push	sp
		push	bp
		push	si
		push	di
		push	ds
		push	es
		mov	bp,sp
		mov	ax,DGROUP
		mov	ds,ax
		assume DS: DGROUP
		cld

		; handler body - start out by restoring stack
		pushf
		cli	

		; save background stack
		mov	word ptr DGROUP:_api_ss,ss
		mov	word ptr DGROUP:_api_sp,sp

		; restore foreground stack here
		mov	ss,word ptr DGROUP:_usr_ss
		mov	sp,word ptr DGROUP:_usr_sp

		; get all the user registers back
		pop	es
		pop	ds
		pop	di
		pop	si
		pop	bp
		pop	bx
		pop	bx
		pop	dx
		pop	cx
		pop	ax

		; do the int 23 handler and see if it returns
		int	23h

		; we're back, must have been users handler
		push	ax
		push	cx
		push	dx
		push	bx
		push	sp
		push	bp
		push	si
		push	di
		push	ds
		push	es
		mov	bp,sp
		mov	ax,DGROUP
		mov	ds,ax
		assume DS: DGROUP

		; test for far return or iret
		cmp	sp,_usr_sp
		jz	hbrk1		; it was far ret

		; restart int 21 from the top
hbrk1:		pop	es
		pop	ds
		pop	di
		pop	si
		pop	bp
		pop	bx
		pop	bx
		pop	dx
		pop	cx
		pop	ax
		call	_int21_entry
		iret

hbrk2:		popf			; clear the flag from the stack
		jnc	hbrk1		; user wants to restart
		pop	es
		pop	ds
		pop	di
		pop	si
		pop	bp
		pop	bx
		pop	bx
		pop	dx
		pop	cx
		pop	ax
		mov	ax,4c00h	; exit
		mov	byte ptr _break_flg,0ffh ; set break detected flag
		call	_int21_entry
		iret

		
_handle_break	endp

ENDIF

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