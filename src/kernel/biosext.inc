COMMENT ^ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   BIOSEXT.INC
   Copyright (C) 1989,1990,1991 by Datalight, All Rights Reserved

   Set BIOSEXT to 1 in ST.INC to include this file.

   NOTE: You must NEVER have both BIOSEXT and POWERUP set TRUE.
   Chose one or the other, but never both.  See ST.INC.
 
   Place this segment first in your location file.  Example:

   [BIOS_EXT] @ 0xC800 +
   CODE       @ $ +
   .
   .
   .

   When this BIOSEXT equate in ST.INC is set to 1, this file
   will be included in the startup code.

   A typical BIOS will look for BIOS extensions between the
   addresses of C0000H and EFFFFH at 2K intervals.  This
   mechanism is used to install new video cards, hard disk cards
   and, in our case, to give our startup code control upon boot.

   This BIOS extension traps INT 19H (the disk boot vector) so
   that it points to our startup code.  That way, when the BIOS
   is finished initializing, it will pass control to our startup
   code (instead of booting the disk).

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ^


; the BIOS boot vector
INT19H	equ	19H * 4

BIOS_EXT	SEGMENT
	ASSUME	CS:BIOS_EXT,DS:nothing

extend_BIOS proc far
extend:
	dw	0AA55H	; BIOS extension id
	db	4	; size of extension in 512 byte chunks (claim 2K)
	jmp	short ex1
	db	0FFH	; byte checksum

ex1:	push	ds	; save BIOS data segment
	mov	ax,0
	mov	ds,ax
	mov	word ptr ds:[INT19H],offset _astart
	mov	word ptr ds:[INT19H+2],seg _astart
	pop	ds
	ret
extend_BIOS endp

BIOS_EXT	ENDS
