#
# makefile for DOS-C boot
#
# $Header:   C:/dos-c/src/boot/boot.mav   1.1   29 Aug 1996 13:06:50   patv  $
#
# $Log:   C:/dos-c/src/boot/boot.mav  $
#
#   Rev 1.1   29 Aug 1996 13:06:50   patv
#Bug fixes for v0.91b
#
#   Rev 1.0   02 Jul 1995  9:11:26   patv
#Initial revision.
#

#
# Uncomment the following for a debug version
#
AFLAGS  =	/zi /DDEBUG
LFLAGS  =	/v

PRODUCT =	boot1440.bin boot720.bin boot360.bin

all:		$(PRODUCT)

production:	all
		copy *.bin ..\..\dist
		copy boot360.bin ..\..\dist\boot.bin
		del *.bin
		del *.map

boot360.bin:	boot.asm
		tasm /DX360 boot,,
		tlink boot
		exe2bin boot boot360.bin
		del boot.obj
		del boot.exe

boot720.bin:	boot.asm
		tasm /DX720 boot,,
		tlink boot
		exe2bin boot boot720.bin
		del boot.obj
		del boot.exe

boot1440.bin:	boot.asm
		tasm /DX1440 boot,,
		tlink boot
		exe2bin boot boot1440.bin
		del boot.obj
		del boot.exe

clean:
		del *.lst
		del *.map
		del *.bin
		del *.bak
		del *.las
		del *.obj
		del *.exe
