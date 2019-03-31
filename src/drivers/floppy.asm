;
; File:
;			  floppy.asm
; Description:
;		    floppy disk driver primitives
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
; $Logfile:   C:/dos-c/src/drivers/floppy.asv  $
;
; $Header:   C:/dos-c/src/drivers/floppy.asv   1.2   29 Aug 1996 13:07:14   patv  $
;
; $Log:   C:/dos-c/src/drivers/floppy.asv  $
;
;   Rev 1.2   29 Aug 1996 13:07:14   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:34   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  7:57:02   patv
;Initial revision.
;

		page	60,132
		title	floppy disk driver primitives

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

_TEXT      segment
	assume	CS: _TEXT
 
;
;
; Reset both the diskette and hard disk system
;
; BOOL fl_reset(VOID)
;
;	returns TRUE if successful
;
  
_fl_reset	proc	near
		public	_fl_reset

		mov	ah,0		; BIOS reset disketter & fixed disk
		int	13h

		jc	fl_rst1		; cy==1 is error
		mov	ax,1		; TRUE on success
		ret

fl_rst1:	xor	ax,ax		; FALSE on error
		ret
_fl_reset	endp
  
  
;
; Read the disk system status
;
; COUNT fl_rd_status(WORD drive)
;
; Returns error codes
;
; See Phoenix Bios Book for error code meanings
;
  
_fl_rd_status	proc	near
		public	_fl_rd_status
		
		push	bp		; C entry
		mov	bp,sp

		mov	dl,[bp+4]	; get the drive number
		mov	ah,1		;  read status
		int	13h

		mov	al,ah		; for the return code
		xor	ah,ah

		pop	bp		; C exit
		ret
_fl_rd_status	endp
  
  
;
; Read Sectors
;
; COUNT fl_read(WORD drive, WORD head, WORD track, WORD sector, WORD count, BYTE FAR *buffer);
;
; Reads one or more sectors.
;
; Returns 0 if successful, error code otherwise.
;
_fl_read	proc	near
		public	_fl_read

		push	bp		; C entry
		mov	bp,sp

		mov	dl,[bp+4]	; get the drive (if or'ed 80h its 
					; hard drive.
		mov	dh,[bp+6]	; get the head number
		mov	ch,[bp+8]	; cylinder number (lo only if hard)
		mov	al,[bp+9h]	; get the top of cylinder
		xor	ah,ah
		mov	cl,6		; form top of cylinder for sector
		shl	ax,cl
		mov	cl,[bp+0Ah]	; sector number
		and	cl,03fh		; mask to sector field bits 5-0
		or	cl,al		; or in bits 7-6
		mov	al,[bp+0Ch]
		les	bx,dword ptr [bp+0Eh]	; Load 32 bit buffer ptr

		mov	ah,2
		int	13h		; read sectors to memory es:bx

		mov	al,ah
		jc	fl_rd1		; error, return error code
		xor	al,al		; Zero transfer count
fl_rd1:
		xor	ah,ah		; force into < 255 count
		pop	bp
		ret
_fl_read	endp
  
  
;
; Write Sectors
;
; COUNT fl_write(WORD drive, WORD head, WORD track, WORD sector, WORD count, BYTE FAR *buffer);
;
; Writes one or more sectors.
;
; Returns 0 if successful, error code otherwise.
;
_fl_write	proc	near
		public	_fl_write

		push	bp		; C entry
		mov	bp,sp

		mov	dl,[bp+4]	; get the drive (if or'ed 80h its 
					; hard drive.
		mov	dh,[bp+6]	; get the head number
		mov	ch,[bp+8]	; cylinder number (lo only if hard)
		mov	al,[bp+9h]	; get the top of cylinder
		xor	ah,ah
		mov	cl,6		; form top of cylinder for sector
		shl	ax,cl
		mov	cl,[bp+0Ah]	; sector number
		and	cl,03fh		; mask to sector field bits 5-0
		or	cl,al		; or in bits 7-6
		mov	al,[bp+0Ch]
		les	bx,dword ptr [bp+0Eh]	; Load 32 bit buffer ptr

		mov	ah,3
		int	13h		;  write sectors from mem es:bx

		mov	al,ah
		jc	fl_rd1		; error, return error code
		xor	al,al		; Zero transfer count
fl_wr1:
		xor	ah,ah		; force into < 255 count
		pop	bp
		ret
_fl_write	endp
  
  
;
;			       SUBROUTINE
;
  
_fl_verify	proc	near
		public	_fl_verify

		push	bp
		mov	bp,sp
		mov	dl,[bp+4]
		mov	dh,[bp+6]
		mov	ch,[bp+8]
		mov	cl,[bp+0Ah]
		mov	al,[bp+0Ch]
		mov	ah,4
		int	13h			; Disk  dl=drive a: ah=func 04h
						;  verify sectors with mem es:bx
		mov	al,ah
		jc	fl_ver1			; Jump if carry Set
		xor	al,al			; Zero register
fl_ver1:
		xor	ah,ah			; Zero register
		pop	bp
		ret
_fl_verify	endp
  
  
_fl_format	proc	near
		public	_fl_format

		xor	ax,ax
		ret
_fl_format	endp
  
  
_TEXT		ends

		end
