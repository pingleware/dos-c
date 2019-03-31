;
; File:
;			     boot.asm
; Description:
;			    DOS-C boot
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
; $Logfile:   C:/dos-c/src/boot/boot.asv  $
;
; $Header:   C:/dos-c/src/boot/boot.asv   1.3   29 Aug 1996 13:06:50   patv  $
;
; $Log:   C:/dos-c/src/boot/boot.asv  $
;	
;	   Rev 1.3   29 Aug 1996 13:06:50   patv
;	Bug fixes for v0.91b
;	
;	   Rev 1.2   01 Sep 1995 17:56:44   patv
;	First GPL release.
;	
;	   Rev 1.1   30 Jul 1995 20:37:38   patv
;	Initialized stack before use.
;	
;	   Rev 1.0   02 Jul 1995 10:57:52   patv
;	Initial revision.
;

		page	60,132
		title	DOS-C boot

; The following equates show data references outside the range of the program.

		DPBSEG	=	00BFh		; buffer for directory search
		BUFSEG	=	0050h		; buffer for directory search
		LOADSEG	=	2000h		; where to load io.sys
		IPL_SIZE=	127		; reserve space
		BIOSSEG	=	07c0h		; where bios loads boot

		DSKBASE	=	1eh*4		; pointer to disk parameters

ipl		segment at LOADSEG

IplEntry	label far

ipl		ends



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
DGROUP		group	_TEXT,_DATA,_BSS,_BSSEND		; tiny model
		assume	cs:_TEXT,ds:DGROUP,ss:DGROUP
_TEXT		ends

_DATA		segment word public 'DATA'
_DATA		ends

_BSS		segment word public 'BSS'
_BSS		ends

_BSSEND		segment byte public 'STACK'
_BSSEND		ends





_BSS		segment word public 'BSS'
dir_sec		dd	1 dup (?)
data_sec	dd	1 dup (?)
cylnum		dw	1 dup (?)
secnum		db	1 dup (?)
loc_dparm	db	11 dup (?)		; local diskette parm storage
dir_buf		dw	512 dup (?)
loc_stk		dw	512 dup (?)
stktop		equ	$
IFDEF DEBUG
buffer		db 16384 dup (?)
ENDIF
_BSS		ends

_TEXT		segment	byte public 'CODE'

		org	0h

dosboot		proc	far

start:
		jmp	short boot
		db	90h
oem		db	'DOSC0.90'

IFDEF X360
;
; all info that follows is for DSDD 9spt 5.25 inch
;

bpb		equ	$

nbytes		dw	512			; bytes/sector
clsiz		db	2			; sectors/allocation unit
nresrvd		dw	1			; # reserved sectors
nfats		db	2			; # of fats
rootsiz		dw	112			; # of root directories
dsksiz		dw	720			; # sectors total in image
disktype	db	0fdh			; media descrip: fd=2side9sec, etc...
fatsiz		dw	2			; # sectors in a fat
numsecs		dw	9			; # sectors/track
numhds		dw	2			; # heads
nhidden		dd	0			; # hidden sectors
nhuge		dd	0			; # sectors if > 65536
ndrive		db	00h			; drive number
nreserved1	db	00h
nsignature	db	29h			; extend boot signature
volid		dd	0
volabel		db	'DOS-C BOOT '
fstype		db	'FAT12   '
ENDIF

IFDEF X720
;
; all info that follows is for DSDD 9spt 3.5 inch
;

bpb		equ	$

nbytes		dw	512			; bytes/sector
clsiz		db	2			; sectors/allocation unit
nresrvd		dw	1			; # reserved sectors
nfats		db	2			; # of fats
rootsiz		dw	112			; # of root directories
dsksiz		dw	1440			; # sectors total in image
disktype	db	0f9h			; media descrip: fd=2side9sec, etc...
fatsiz		dw	3			; # sectors in a fat
numsecs		dw	9			; # sectors/track
numhds		dw	2			; # heads
nhidden		dd	0			; # hidden sectors
nhuge		dd	0			; # sectors if > 65536
ndrive		db	00h			; drive number
nreserved1	db	00h
nsignature	db	29h			; extend boot signature
volid		dd	0
volabel		db	'DOS-C BOOT '
fstype		db	'FAT12   '
ENDIF

IFDEF X1440
;
; all info that follows is for 3.5 inch 1.44M
;

bpb		equ	$

nbytes		dw	512			; bytes/sector
clsiz		db	1			; sectors/allocation unit
nresrvd		dw	1			; # reserved sectors
nfats		db	2			; # of fats
rootsiz		dw	0e0h			; # of root directories
dsksiz		dw	0b40h			; # sectors total in image
disktype	db	0f0h			; media descrip: fd=2side9sec, etc...
fatsiz		dw	9			; # sectors in a fat
numsecs		dw	18			; # sectors/track
numhds		dw	2			; # heads
nhidden		dd	0			; # hidden sectors
nhuge		dd	0			; # sectors if > 65536
ndrive		db	00h			; drive number
nreserved1	db	00h
nsignature	db	29h			; extend boot signature
volid		dd	0
volabel		db	'DOS-C BOOT '
fstype		db	'FAT12   '
ENDIF

IFDEF DOSEMU
nbytes		dw	512			; bytes/sector
clsiz		db	8			; sectors/allocation unit
nresrvd		dw	1			; # reserved sectors
nfats		db	2			; # of fats
rootsiz		dw	512			; # of root directories
dsksiz		dw	2431			; # sectors total in image
disktype	db	0f8h			; media descrip: fd=hd
fatsiz		dw	1			; # sectors in a fat
numsecs		dw	17			; # sectors/track
numhds		dw	4			; # heads
nhidden		dd	17			; # hidden sectors
nhuge		dd	0			; # sectors if > 65536
ndrive		db	80h			; drive number
nreserved1	db	00h
nsignature	db	29h			; extend boot signature
volid		dd	0
volabel		db	'DOS-C BOOT '
fstype		db	'FAT12   '
ENDIF


boot:
		;
		; We need the following little bit of trickery so that we
		; can save space later. We know that we are booted to
		; 0000:7c00, but the traditional DOS developement tools
		; cannot create a binary image that starts at 7c00h.  To get
		; around it, we will change to 07c0:0000 instead, and use
		; that address instead. Now we can org at 0 and load all
		; registers.  Also, this makes us somewhat position 
		; independant.
IFDEF DEBUG
		;
		; However, in order to debug this code under a debugger, we
		; need to simulate the technique.
		;
		cli				; Disable interrupts
		mov	ax,seg boot0		; We'll pop cs:ip through
		push	ax			; a far return
		mov	ax,offset _TEXT:boot0
		push	ax
		retf
ELSE
		cli				; Disable interrupts
		mov	ax,BIOSSEG		; Initialize the stack segment
		mov	ss,ax			; We'll pop cs:ip through
		mov	sp,offset _TEXT:stktop
		push	ax			; a far return
		mov	ax,offset _TEXT:boot0
		push	ax
		retf
ENDIF
boot0:
		push	cs			; fix segment regs
		pop	ss			; to tiny model
		mov	sp,offset DGROUP:stktop	; Fix stack pointer
		push	ss			; and set es=ss
		pop	es
		xor	ax,ax			; Diskette param segment = 0
		mov	ds,ax
		mov	bx,DSKBASE		; Diskette param pointer
		lds	si,dword ptr ss:[bx]	; get 32 bit Diskette param
		push	ds			; save it, in case of reboot
		push	si
		push	ds:[si]
		push	ds:[si+2]
		mov	di,word ptr loc_dparm	; copy it
		mov	cx,0Bh			; 11 bytes worth of data
		cld				; Clear direction
		rep	movsb			; Mov ds:[si] to es:[di]
		mov	byte ptr [di-2],0Fh	; change head settle to 15ms
		mov	cx,cs:numsecs		; fix dparm sec per track
		mov	[di-7],cl
		mov	[bx+2],ax		; save new dparm seg and offset
		mov	word ptr [bx],offset DGROUP:boot
		sti				; Enable interrupts
IFNDEF DEBUG
		int	13h			; Disk  dl=drive a  ah=func 00h
						;  reset disk, al=return status
ENDIF
		push	es			; Now set ds=es=ss
		pop	ds
		jc	booterr			; error if carry Set
		mov	si,offset DGROUP:bootm	; Print boot message
		call	printmsg
		xor	ax,ax			; Zero register
		cmp	dsksiz,ax		; if disksiz == 0
		je	boot1			; continue
		mov	cx,dsksiz		; else make
		mov	word ptr nhuge,cx	; nhuge = dsksiz
boot1:
		mov	al,nfats		; Compute location of root
		mul	word ptr fatsiz		; fatsiz * nfats + nhidden
						; + nresrvd
		add	ax,word ptr nhidden
		adc	dx,word ptr nhidden+2
		add	ax,nresrvd
		adc	dx,0
		mov	word ptr dir_sec,ax
		mov	word ptr dir_sec+2,dx
		mov	word ptr data_sec,ax
		mov	word ptr data_sec+2,dx
		mov	ax,20h			; compute root size in secs
		mul	word ptr rootsiz	; rootsiz entries * 32 bytes
		mov	bx,nbytes		; round it up
		add	ax,bx			; by addings nbytes - 1

		dec	ax
		div	bx			; and dividing by nbytes
		add	word ptr data_sec,ax	; dir_sec + rootsiz = 1st data
		adc	word ptr data_sec+2,0
		mov	bx,offset DGROUP:dir_buf
		mov	dx,word ptr dir_sec+2	; Now begin dir search
		mov	ax,word ptr dir_sec
		call	calc_abs
		jc	booterr			; Jump if can't read dir
		mov	al,1
		call	diskop			; Read the sector
		jc	booterr			; Jump if can't read
		mov	di,bx
		mov	cx,0Bh
		mov	si,offset DGROUP:ipl_name	; compare ipl name first
		repe	cmpsb
		jz	boot2			; Jump if no match
booterr:
		mov	si,offset DGROUP:err1	; Print error message
		call	printmsg
		xor	ax,ax			; and get keybd char in al
		int	16h			; so that we can re-boot
		pop	si			; put back old dparms
		pop	ds
		pop	word ptr [si]
		pop	word ptr [si+2]
		int	19h			; Bootstrap loader (re-boot)
booterr1:
		jmp	short booterr		; loop if we fell through
boot2:
		mov	ax,[bx+1Ah]		; get the start cluster
		dec	ax
		dec	ax
		mov	bl,clsiz		; convert it to sector
		xor	bh,bh
		mul	bx
		add	ax,word ptr data_sec	; add to start of data area
		adc	dx,word ptr data_sec+2	; for start of file in sectors
IFDEF DEBUG
		mov	bx,offset DGROUP:buffer	; point where to load
ELSE
		mov	bx,LOADSEG		; point to load segment
		mov	es,bx
		xor	bx,bx			; then offset
ENDIF
		mov	cx,IPL_SIZE		; and how much

boot3:
		push	ax			; save pacer variables
		push	dx
		push	cx
		call	calc_abs		; compute sector
		jc	booterr1		; Jump if error
		mov	al,1			; set to read 1 sector

		call	diskop			; and do it
		pop	cx			; recover pacer variables
		pop	dx
		pop	ax
		jc	booterr			; Jump if error
		add	ax,1			; increment sector number
		adc	dx,0
		add	bx,nbytes
		loop	boot3			; Loop if cx > 0 (any left)

		xor	bh,bh
		mov	bl,ndrive
IFNDEF DEBUG
;		jmp	IplEntry
		db	0eah
		dw	0
		dw	LOADSEG
ENDIF

dosboot		endp

;
; printmsg
;	Print an asciz string pointed to by ds:si
;
printmsg	proc	near
		lodsb				; String [si] to al
		or	al,al			; Zero ?
		jz	loc_ret			; Jump if zero
		mov	ah,0Eh
		mov	bx,7
		int	10h			; Video display   ah=functn 0Eh
						;  write char al, teletype mode
		jmp	short printmsg		; (0152)
printmsg	endp

;
; calc_abs
;	Compute the physical disk parameters given a logical disk sector
;	number.
;
;		ax = logical sector number
calc_abs	proc near
		cmp	ax,dsksiz		; see if in range
		jae	calc_err		; exit if not
		div	word ptr numsecs	; target sector / sectors per track
		inc	dl			; sectors start at 1, not 0
		mov	secnum,dl		; and save it for read
		xor	dx,dx			; dx:ax has track number
		div	word ptr numhds		; div by # of heads on media
		mov	nreserved1,dl		; save head number computed
		mov	cylnum,ax		; and the clyinder number
		clc				; Clear carry flag
		retn				; succes return
calc_err:
		stc				; Set carry flag
						; and do error return
loc_ret:

		retn
calc_abs	endp


;
; diskop:
;	Do a read operation
;
diskop		proc	near
		mov	ah,2			; build the vars
		mov	dx,cylnum
		mov	cl,6
		shl	dh,cl
		or	dh,secnum
		mov	cx,dx
		xchg	ch,cl
		mov	dl,ndrive
		mov	dh,nreserved1
		int	13h		;do the bios disk op
		retn
diskop		endp

bootm		db	0Dh, 0Ah, 'BOOT', 0
err1		db	' ERROR: '
		db	'Replace disk and press any key'
		db	0Dh, 0Ah
		db	0
ipl_name	db	'IPL     SYS'
end_boot label byte
		db	((512-2) - (end_boot-start)) dup (0) ; pad with zeros.
		org	1feh
		db	 55h,0AAh

_TEXT		ends



		end	start

