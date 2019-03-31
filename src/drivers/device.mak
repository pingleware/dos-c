#
# makefile for device.lib
#
# $Header:   C:/dos-c/src/drivers/device.mav   1.2   29 Aug 1996 13:07:12   patv  $
#
# $Log:   C:/dos-c/src/drivers/device.mav  $
#
#   Rev 1.2   29 Aug 1996 13:07:12   patv
#Bug fixes for v0.91b
#
#   Rev 1.1   19 Feb 1996  3:19:26   patv
#Added NLS, int2f and config.sys processing
#
#   Rev 1.0   02 Jul 1995  7:54:52   patv
#Initial revision.
#

# MICROSOFT C
# -----------
#MODEL  = s
#CC     = CL
#CFLAGS = /c /Gs /A$(MODEL)
#ASM    = MASM
#AFLAGS = /Mx /Dmem$(MODEL)=1
#LIB    = LIB
#TERM   = ;

# BORLAND C
# -----------
MODEL  = s
CC     = bc
CFLAGS = -c -m$(MODEL)
ASM    = tasm
AFLAGS = /Mx /Dmem$(MODEL)=1
LIB    = tlib
LIBFLAGS = /c
TERM   =

OBJS   = console.obj devend.obj eoi.obj floppy.obj fmemcmp.obj fmemcpy.obj \
fmemset.obj inb.obj int86.obj intr.obj inw.obj keyboard.obj outb.obj \
outw.obj setvec.obj getvec.obj spl.obj timer.obj rdpcclk.obj rdatclk.obj \
wrpcclk.obj wratclk.obj

#LIBOBJS1= +console +devend +eoi +floppy +fmemcmp +fmemcpy
#LIBOBJS2= +fmemset +inb +int86 +intr +inw +keyboard +outb
#LIBOBJS3= +outw +setvec +getvec +spl +timer +rdpcclk +rdatclk
#LIBOBJS4= +wrpcclk +wratclk


LIBOBJS1= +console +devend +eoi +floppy +fmemcmp +fmemcpy
LIBOBJS2= +fmemset +inb +int86 +inw +keyboard +outb
LIBOBJS3= +outw +setvec +getvec +spl +timer +rdpcclk +rdatclk
LIBOBJS4= +wrpcclk +wratclk



# Build the LIBRARY
# -----------------
all:		production

production:	device.lib
		copy device.lib ..\..\lib

clean:
		del device.lib
		del *.obj
		del *.bak
		del *.crf
		del *.xrf
		del *.map
		del *.lst

device.lib : $(OBJS)
	DEL device.lib
	$(LIB) $(LIBFLAGS) device $(LIBOBJS1) $(TERM)
	$(LIB) $(LIBFLAGS) device $(LIBOBJS2) $(TERM)
	$(LIB) $(LIBFLAGS) device $(LIBOBJS3) $(TERM)
	$(LIB) $(LIBFLAGS) device $(LIBOBJS4) $(TERM)



# RULES (DEPENDENCIES)
# ----------------
.c.obj :
	$(CC) $(CFLAGS) $<

.asm.obj :
	$(ASM) $(AFLAGS) $< ;

