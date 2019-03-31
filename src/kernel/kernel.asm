;
; File:
;			   kernel.asm
; Description:
;			kernel start-up code
;
;		     Copyright (c) 1995, 1996
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
; $Logfile:   D:/dos-c/src/kernel/kernel.asv  $
;
; $Header:   D:/dos-c/src/kernel/kernel.asv   1.3   29 May 1996 21:03:44   patv  $
;
; $Log:   D:/dos-c/src/kernel/kernel.asv  $
;
;   Rev 1.3   29 May 1996 21:03:44   patv
;bug fixes for v0.91a
;
;   Rev 1.2   19 Feb 1996  3:24:06   patv
;Added NLS, int2f and config.sys processing
;
;   Rev 1.1   01 Sep 1995 17:54:24   patv
;First GPL release.
;
;   Rev 1.0   02 Jul 1995  9:05:44   patv
;Initial revision.
;

		page	60,132
		title	kernel start-up code

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


_DATA		segment word public 'DATA'
		db '$Header:   D:/dos-c/src/kernel/kernel.asv   1.3   29 May 1996 21:03:44   patv  $'
_DATA		ends

_TEXT		segment	byte public 'CODE'
		assume	cs:_TEXT

		extrn	_main:near
		extrn	_con_driver:near
		extrn	_blk_driver:near
		extrn	_clk_driver:near

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

IFNDEF ??version
 IF STANDALONE EQ 1
PUBLIC	__acrtused 		; trick used by MSC to force in startup
	__acrtused = 9876h
 ENDIF
ENDIF
		;
		;
		page
		;
		;
entry		proc	near
IF STANDALONE EQ 1
		jmp	short kernel_start
ENDIF

		;
		; The "CON" device
		;
		; This device is the standard console device used by
		; XDOS and kernel
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
		db	4
		db	0,0,0,0,0,0,0

		;
		; Header for device
		;
		public	_clk_dev
_clk_dev	label	far
		dd	-1
		dw	8004h		; clock device
		dw	offset clk_strategy
		dw	offset clk_entry
		db	'CLOCK$  '

		page
		;
		; kernel start-up
		;
IF STANDALONE EQ 1
kernel_start:	cli			; prevent interrupts while starting
		mov	ax,DGROUP
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
ENDIF
entry		endp


		;
		; _exit
		;	perform an "exit" and quit
		;
		; exit(code)
		; int code;
		;
IF STANDALONE EQ 1
_exit		proc	near
		public	_exit
		
		cli
		hlt
		jmp	_exit

_exit		endp
ENDIF
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
		mov	ax,DGROUP			; correct for segments
		mov	ds,ax				; ax to carry segment
		mov	word ptr DGROUP:con_dos_stk,sp	; use internal stack
		mov	word ptr DGROUP:con_dos_seg,ss
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
		mov	sp,word ptr DGROUP:con_dos_stk	; use dos stack
		mov	ss,word ptr DGROUP:con_dos_seg
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
		mov	ax,DGROUP			; small model - cs != ds
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
		mov	ax,DGROUP			; correct for segments
		mov	ds,ax				; ax to carry segment
		mov	word ptr DGROUP:blk_dos_stk,sp	; use internal stack
		mov	word ptr DGROUP:blk_dos_seg,ss
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
		mov	sp,word ptr DGROUP:blk_dos_stk	; use dos stack
		mov	ss,word ptr DGROUP:blk_dos_seg
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
		; clock device strategy
		;
		; NOTE: This code is not standard device driver handlers
		; It is written for sperate code and data space.
		;
clk_strategy	proc	far
		push	ds
		push	ax
		; small model
		mov	ax,DGROUP			; small model - cs != ds
		mov	ds,ax
		mov	word ptr DGROUP:clk_rp+2,es
		mov	word ptr DGROUP:clk_rp,bx
		pop	ax
		pop	ds
		ret	
clk_strategy	endp

		;
		; clock device interrupt
		;
		; NOTE: This code is not standard device driver handlers
		; It is written for sperate code and data space.
		;
clk_entry	proc	far
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
		mov	ax,DGROUP			; correct for segments
		mov	ds,ax				; ax to carry segment
		mov	word ptr DGROUP:clk_dos_stk,sp	; use internal stack
		mov	word ptr DGROUP:clk_dos_seg,ss
		pushf					; put flags in bx
		pop	bx
		cli					; no interrupts
		mov	ss,ax
		mov	sp,offset DGROUP:clk_stk_top
		push	bx
		popf					; restore interrupt flag
		mov	bp,sp				; make a c frame
		push	word ptr clk_rp+2
		push	word ptr clk_rp
		call	_clk_driver
		pop	cx
		pop	cx
		les	bx,dword ptr clk_rp		; now return completion code
		mov	word ptr es:[bx].status,ax	; mark operation complete
		cli					; no interrupts
		mov	sp,word ptr DGROUP:clk_dos_stk	; use dos stack
		mov	ss,word ptr DGROUP:clk_dos_seg
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
clk_entry	endp


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
;_nul_dev	label	far
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
rqhdr		dd	(?)
con_rp		dd	(?)
blk_rp		dd	(?)
clk_rp		dd	(?)
con_dos_stk	dw	(?)
con_dos_seg	dw	(?)
blk_dos_stk	dw	(?)
blk_dos_seg	dw	(?)
clk_dos_stk	dw	(?)
clk_dos_seg	dw	(?)
intr_dos_stk	dw	(?)
intr_dos_seg	dw	(?)


		public	_api_sp
_api_sp		dw	(?)		; api stacks - for context
		public	_api_ss
_api_ss		dw	(?)		; switching
		public	_usr_sp
_usr_sp		dw	(?)		; user stacks
		public	_usr_ss
_usr_ss		dw	(?)
		public	_ram_top
_ram_top	dw	(?)

_BSS		ends



_BSSEND		segment byte public 'STACK'
; kernel private stack arear
IF STANDALONE EQ 1
		dw	512 dup (?)
ELSE
		dw	4096 dup (?)
ENDIF
tos		label	byte

; condev private stack
		dw	1024 dup (?)
con_stk_top	label	byte

; blockdev private stack
		dw	1024 dup (?)
blk_stk_top	label	byte

; clockdev private stack
		dw	1024 dup (?)
clk_stk_top	label	byte

; interrupt stack
		dw	1024 dup (?)
intr_stk_top	label	byte

; int 21 api stack area
		dw	1024 dup (?)
		public	_error_tos
_error_tos	label	word
		dw	4096 dup (?)
		public	_char_api_tos
_char_api_tos	label	word
		dw	4096 dup (?)
		public	_disk_api_tos
_disk_api_tos	label	word
		public	last
last		label	word			; must always be end of stack area
		public	_last
_last		label	byte			; and c version

_BSSEND		ends


		end	entry

