;
; File:
;			     eoi.asm
; Description:
;		    end of interrupt primitive
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
; $Logfile:   C:/dos-c/src/drivers/eoi.asv  $
;
; $Header:   C:/dos-c/src/drivers/eoi.asv   1.2   29 Aug 1996 13:07:12   patv  $
;
; $Log:   C:/dos-c/src/drivers/eoi.asv  $
;
;   Rev 1.2   29 Aug 1996 13:07:12   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:36   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  7:57:26   patv
;Initial revision.
;

		page	60,132
		title	eoi - end of interrupt primitive

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
_eoi		proc	near
		public	_eoi
  
		mov	al,20h
		out	20h,al			; port 20h, 8259-1 int command
						;  al = 20h, end of interrupt
		ret
  
_eoi		endp
  
_TEXT		ends

		end
