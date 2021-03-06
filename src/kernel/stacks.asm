;
; File:
;			  stacks.asm
; Description:
;     Assembly support routines for stack manipulation, etc.
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
; $Logfile:   D:/dos-c/src/kernel/stacks.asv  $
;
; $Header:   D:/dos-c/src/kernel/stacks.asv   1.2   29 May 1996 21:03:36   patv  $
;
; $Log:   D:/dos-c/src/kernel/stacks.asv  $
;
;   Rev 1.2   29 May 1996 21:03:36   patv
;bug fixes for v0.91a
;
;   Rev 1.1   01 Sep 1995 17:54:24   patv
;First GPL release.
;
;   Rev 1.0   02 Jul 1995  9:05:50   patv
;Initial revision.
;

		page	60,132
		title	Assembly support routines for stack manipulation, etc.

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



		extrn	_api_sp:word		; api stacks - for context
		extrn	_api_ss:word		; switching
		extrn	_usr_sp:word		; user stacks
		extrn	_usr_ss:word

_TEXT      	segment
		assume	cs: _TEXT
		public	_user_stack
;
; void far user_stack(void) - 
;	switch to user stack area
;
_user_stack	proc	far

		; save foreground stack

		; we need to get the return values from the stack 
		; since the current stack will change
		pop	ax			;get return offset
		pop	bx			;get return segment

		; Save the flags so that we can restore correct interrupt
		; state later. We need to disable interrupts so that we
		; don't trash memory with new sp-old ss combination
		pushf
		pop	dx
		cli

		; save away foreground process' stack
		mov	word ptr DGROUP:_usr_ss,ss
		mov	word ptr DGROUP:_usr_sp,sp

		; setup our local stack
		mov	ss,word ptr DGROUP:_api_ss
		mov	sp,word ptr DGROUP:_api_sp

		; make bp relative to our stack frame
		mov	bp,sp

		; setup for ret
		push	bx  
		push	ax

		; now restore interrupt state
		push	dx
		popf

		ret

_user_stack	endp

;
; void far kernel_stack(void) - 
;	restore foreground stack, throw ours away 
;
		public	_kernel_stack
_kernel_stack  proc    far

	; we need to get the return values from the stack 
	; since the current stack will change
		pop	cx			;get return offset
		pop	bx			;get return segment

		; Save the flags so that we can restore correct interrupt
		; state later. We need to disable interrupts so that we
		; don't trash memory with new sp-old ss combination
		pushf
		pop	dx
		cli

		; save background stack
		mov	word ptr DGROUP:_api_ss,ss
		mov	word ptr DGROUP:_api_sp,sp

		; restore foreground stack here
		mov	ss,word ptr DGROUP:_usr_ss
		mov	sp,word ptr DGROUP:_usr_sp

		; make bp relative to our stack frame
		mov	bp,sp   

		; setup for ret
		push	bx 
		push	cx
            
		; now restore interrupt state
		push	dx
		popf

		ret
_kernel_stack  endp

_TEXT		ends

		end

