;
; File:
;			    ipl.asm
; Description:
;			ipl start-up code
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
; $Logfile:   C:/dos-c/src/ipl/ipl.asv  $
;
; $Header:   C:/dos-c/src/ipl/ipl.asv   1.3   29 Aug 1996 13:06:50   patv  $
;
; $Log:   C:/dos-c/src/ipl/ipl.asv  $
;
;   Rev 1.3   29 Aug 1996 13:06:50   patv
;Bug fixes for v0.91b
;
;   Rev 1.2   01 Sep 1995 17:44:40   patv
;First GPL release.
;
;   Rev 1.1   30 Jul 1995 20:48:12   patv
;Eliminated version strings in ipl
;
;   Rev 1.0   02 Jul 1995  8:52:04   patv
;Initial revision.
;

		page	60,132
		title	ipl start-up code

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

		extrn	_main:near
		extrn	_con_driver:near
		extrn	_blk_driver:near

;STANDALONE equ 0
		

;---------------------------------------------------
;
; Device entry points
;
cmdlen	equ	0			; Length of this command
unit	equ	1			; Subunit Specified
cmd	equ	2			; Command Code
status	equ	3			; Status
media	equ	13			; Media Descriptor
trans	equ	14			; Transfer Address
count	equ	18			; Count of blocks or characters
start	equ	20			; First block to transfer

		;
		;
		page
		;
		;
entry		proc	near
		jmp	short ipl_start

		;
		; The "CON" device
		;
		; This device is the standard console device used by
		; XDOS and ipl
		;
		public	_con_dev
_con_dev	label	far
		dd	-1
		dw	8003h		; con device (stdin & stdout)
		dw	offset con_strategy
		dw	offset con_entry
		db	'CON     '

		;
		; Header for device
		;
		public	_blk_dev
_blk_dev	label	far
		dd	-1
		dw	0000h		; block device
		dw	offset blk_strategy
		dw	offset blk_entry
		db	3
		db	0,0,0,0,0,0,0

		page
		;
		; ipl start-up
		;
ipl_start:	cli			; prevent interrupts while starting
		mov	bx,ax		; save the boot type
		mov	ax,cs		; initialize the segment registers
		mov	ds,ax
		mov	es,ax
		mov	ax,seg DGROUP:tos
		mov	ss,ax
		mov	sp,offset DGROUP:tos
		mov	ds,ax
		mov	es,ax
		mov	bp,sp		; and set up stack frame for c
		sti			; now enable them
		mov	_BootDrive,bx	; tell where we came from
		mov	_NumFloppies,cx	; and how many
		call	_main
		mov	ax,0
		push	ax
		call	_exit
		jmp	$
entry		endp


		;
		; _exit
		;	perform an "exit" and quit
		;
		; exit(code)
		; int code;
		;
_exit		proc	near
		public	_exit
		
		cli
		hlt
		jmp	_exit

_exit		endp

		;
		; _MarkStack
		;	Take a snapshot of the current stack pointer.
		;	Segmented stack in MarkSS and MarkSP.
		;
		; NOTE:
		;	You need at least two more byts of stack space
		;	other than what is reported to support this
		;	call.
		;
		; void MarkStack(void)
		;
_MarkStack	proc	near
		public	_MarkStack

		mov	ax,ss			; get the stack segment
		mov	_MarkSS,ax
		mov	ax,sp			; and the stack pointer
		add	ax,2			; adjust for this call
		mov	_MarkSP,ax
		ret
_MarkStack	endp

		page
		;
		; NUL device strategy
		;
_nul_strtgy	proc	far
		public _nul_strtgy
		mov	word ptr rqhdr,bx	;save rq headr
		mov	word ptr rqhdr+2,es
		ret
_nul_strtgy	endp

		;
		; NUL device interrupt
		;
_nul_intr	proc	far
		public _nul_intr
		push	es
		push	bx
		les	bx,rqhdr		;es:bx--> rqheadr
		or	word ptr es:[bx+3],100h	;set "done" flag
		pop	bx
		pop	es
		ret
_nul_intr	endp

 		page
		;
		; con device strategy
		;
		; NOTE: This code is not standard device driver handlers
		; It is written for sperate code and data space.
		;
con_strategy	proc	far
		push	ds
		push	ax
		; small model
		mov	ax,DGROUP		; small model - cs != ds
		mov	ds,ax
		mov	word ptr DGROUP:con_rp+2,es
		mov	word ptr DGROUP:con_rp,bx
		pop	ax
		pop	ds
		ret	
con_strategy	endp


		;
		; con device interrupt
		;
		; NOTE: This code is not standard device driver handlers
		; It is written for sperate code and data space.
		;
con_entry	proc	far
		push	si
		push	ax
		push	cx
		push	dx
		push	di
		push	bp
		push	ds
		push	es
		push	bx

		; small model
		mov	ax,DGROUP		; small model - cs != ds
		mov	ds,ax				; ax to carry segment
		mov	word ptr DGROUP:dos_stk,sp	; use internal stack
		mov	word ptr DGROUP:dos_seg,ss
		pushf					; put flags in bx
		pop	bx
		cli					; no interrupts
		mov	ss,ax
		mov	sp,offset DGROUP:con_stk_top
		push	bx
		popf					; restore interrupt flag
		mov	bp,sp				; make a c frame
		push	word ptr con_rp+2
		push	word ptr con_rp
		call	_con_driver
		pop	cx
		pop	cx
		les	bx,dword ptr con_rp		; now return completion code
		mov	word ptr es:[bx].status,ax	; mark operation complete
		pushf
		pop	bx
		cli					; no interrupts
		mov	sp,word ptr DGROUP:dos_stk	; use dos stack
		mov	ss,word ptr DGROUP:dos_seg
		push	bx
		popf					; restore interrupt flag
		pop	bx
		pop	es
		pop	ds
		pop	bp
		pop	di
		pop	dx
		pop	cx
		pop	ax
		pop	si
		ret	
con_entry	endp

		page
		;
		; block device strategy
		;
		; NOTE: This code is not standard device driver handlers
		; It is written for sperate code and data space.
		;
blk_strategy	proc	far
		push	ds
		push	ax
		; small model
		mov	ax,DGROUP		; small model - cs != ds
		mov	ds,ax
		mov	word ptr DGROUP:blk_rp+2,es
		mov	word ptr DGROUP:blk_rp,bx
		pop	ax
		pop	ds
		ret	
blk_strategy	endp

		;
		; block device interrupt
		;
		; NOTE: This code is not standard device driver handlers
		; It is written for sperate code and data space.
		;
blk_entry	proc	far
		pushf
		push	ax
		push	bx
		push	cx
		push	dx
		push	bp
		push	si
		push	di
		push	ds
		push	es

		; small model
		mov	ax,DGROUP			; small model - cs != ds
		mov	ds,ax				; ax to carry segment
		mov	word ptr DGROUP:dos_stk,sp	; use internal stack
		mov	word ptr DGROUP:dos_seg,ss
		pushf					; put flags in bx
		pop	bx
		cli					; no interrupts
		mov	ss,ax
		mov	sp,offset DGROUP:blk_stk_top
		push	bx
		popf					; restore interrupt flag
		mov	bp,sp				; make a c frame
		push	word ptr blk_rp+2
		push	word ptr blk_rp
		call	_blk_driver
		pop	cx
		pop	cx
		les	bx,dword ptr blk_rp			; now return completion code
		mov	word ptr es:[bx].status,ax	; mark operation complete
		cli					; no interrupts
		mov	sp,word ptr DGROUP:dos_stk	; use dos stack
		mov	ss,word ptr DGROUP:dos_seg
		pop	es
		pop	ds
		pop	di
		pop	si
		pop	bp
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		popf
		ret	
blk_entry	endp


		page

;
; special interrupt routine for break key handling
;


;
; i_save
;	save machine context on stack after an interrupt
;	does not save ss and sp
;
;	assumes flags and cs:ip are on stack from interrupt
;
i_save	proc	near
	public	i_save
	push	bp		; start saving bp
	mov	bp,sp		; and mark frame for exit
	push	di		; save the rest of the registers
	push	si
	push	bx
	push	cx
	push	dx
	push	ds
	push	es
	xchg	ax,[bp+2]	; swap return address with ax
	push	ax		; put it back
	mov	ax,[bp+2]	; recover ax from entry
	mov	bp,[bp]		; and bp
	ret
i_save	endp


;
; i_exit
;	recovers context from i_save and does interrupt return
;	must be jumped to
;
;	warning - make sure you're using the right stack
;
i_exit	proc	near
	public	i_exit
	pop	es		; recover context
	pop	ds
	pop	dx
	pop	cx
	pop	bx
	pop	si
	pop	di
	pop	bp
	pop	ax
	iret			; and make like a tree (leave)
i_exit	endp


	extrn	_break_handler:near

_break_key	proc	near
		public	_break_key
		call	i_save			; save context
		mov	ax,cs			; correct for segments
		mov	ds,ax			; ax to carry segment
		mov	word ptr DGROUP:intr_dos_stk,sp	; use internal stack
		mov	word ptr DGROUP:intr_dos_seg,ss
		pushf				; put flags in bx
		pop	bx
		cli				; no interrupts
		mov	ss,ax
		mov	sp,offset DGROUP:intr_stk_top
		push	bx
		popf				; restore interrupt flag
		mov	bp,sp			; make a c frame
		call	_break_handler		; call handler
		les	bx,dword ptr con_rp	; now return completion code
		pushf
		pop	bx
		cli				; no interrupts
		mov	sp,word ptr DGROUP:intr_dos_stk	; use dos stack
		mov	ss,word ptr DGROUP:intr_dos_seg
		push	bx
		popf
		jmp	i_exit			; and restore & exit
_break_key	endp

   ;	
   ;	VOID INRPT FAR handle_break()
   ;	
	assume	cs:_TEXT
		public	_handle_break
_handle_break	proc	far
		push	ax
		push	bx
		push	cx
		push	dx
		push	es
		push	ds
		push	si
		push	di
		push	bp
		mov	bp,cs:DGROUP
		mov	ds,bp
		mov	bp,sp
		; do nothing
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
_handle_break	endp

_TEXT		ends


_DATA		segment word public 'DATA'
; Globally referenced variables - WARNING: DO NOT CHANGE ORDER
; BECAUSE THEY ARE DOCUMENTED AS UNDOCUMENTED (?) AND HAVE
; MANY MULTIPLEX PROGRAMS AND TSR'S ACCESSING THEM
		public	_first_mcb
_first_mcb	dw	(?)		; Start of user memory
		public	_DPBp
_DPBp		dd	(?)		; First drive Parameter Block
		public	_sfthead
_sfthead	dd	(?)		; System File Table head
		public	_clock
_clock		dd	(?)		; CLOCK$ device
		public	_syscon
_syscon		dd	(?)		; console device
		public	_maxbksize
_maxbksize	dw	(?)		; Number of Drives in system
		public	_firstbuf;
_firstbuf	dd	(?)	    	; head of buffers linked list
		public	_CDSp
_CDSp		dd	(?)		; Current Directory Structure
		public	_FCBp
_FCBp		dd	(?)		; FCB table pointer
		public	_nprotfcb
_nprotfcb	dw	(?)		; number of protected fcbs
		public	_nblkdev
_nblkdev	db	(?)		; number of block devices
		public	_lastdrive
_lastdrive	db	(?)		; value of last drive
		public	_nul_dev
_nul_dev	label	dword
		dd	-1
		dw	8004h		; attributes = char device, NUL bit set
		dw	offset _nul_strtgy
		dw	offset _nul_intr
		db	'NUL     '
		public	_njoined
_njoined	db	(?)		; number of joined devices
;
; end of controlled variables
;
_DATA		ends

_BSS		segment word public 'BSS'
		public	_BootDrive
_BootDrive	dw	(?)
		public	_NumFloppies
_NumFloppies	dw	(?)
_MarkSS		dw	(?)
		public	_MarkSS
_MarkSP		dw	(?)
		public	_MarkSP
rqhdr		dd	(?)
con_rp		dd	(?)
blk_rp		dd	(?)
clk_rp		dd	(?)
dos_stk		dw	(?)
dos_seg		dw	(?)
intr_dos_stk	dw	(?)
intr_dos_seg	dw	(?)

; condev private stack
		dw	1024 dup (?)
con_stk_top	label	byte
		public	con_stk_top

; blockdev private stack
		dw	1024 dup (?)
		dw	1024 dup (?)
blk_stk_top	label	byte
		public	blk_stk_top

; interrupt stack
		dw	1024 dup (?)
intr_stk_top	label	byte
		public	intr_stk_top

; ipl private stack area
;		dw	3072 dup (?)
		dw	8192 dup (?)
tos		label	byte
		public	tos

_BSS		ends




_BSSEND		segment byte public 'STACK'

		public	last
last		label	word			; must always be end of stack area
		public	_last
_last		label	byte			; and c version

_BSSEND		ends


		end


