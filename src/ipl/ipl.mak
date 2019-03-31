#
# Makefile for Borland C++ 3.1 - ipl.sys
#
# $Header:   C:/dos-c/src/ipl/ipl.mav   1.1   29 Aug 1996 13:06:48   patv  $
#
# $Log:   C:/dos-c/src/ipl/ipl.mav  $
#
#   Rev 1.1   29 Aug 1996 13:06:48   patv
#Bug fixes for v0.91b
#
#   Rev 1.0   02 Jul 1995  8:25:36   patv
#Initial revision.
#

RELEASE = 0.91b

.AUTODEPEND

#
# Compiler and Options for Borland C++
# ------------------------------------
CC = bcc +ipl.cfg
ASM = TASM
LIB = TLIB
LINK = TLINK
CFLAGS=-ms -v -ID:\BORLANDC\INCLUDE -LD:\BORLANDC\LIB -DIPL=1 -DI86=1 -DPROTO=1
AFLAGS=/mx/zi
LIBPATH = .
INCLUDEPATH = ..\..\HDR
LIBS		=..\..\LIB\DEVICE.LIB ..\..\LIB\LIBM.LIB

# where to copy source from
FSSRC = \
  ..\fs\fatfs.c \
  ..\fs\fatdir.c \
  ..\fs\fattab.c \
  ..\fs\dosfns.c \
  ..\fs\dosnames.c \
  ..\fs\error.c
SUPTSRC = \
  ..\fs\prf.c \
  ..\fs\misc.c \
  ..\fs\syspack.c \
  ..\kernel\task.c \
  ..\kernel\memmgr.c \
  ..\kernel\strings.c
IOSRC = \
  ..\fs\blockio.c \
  ..\fs\chario.c \
  ..\kernel\dsk.c \
  ..\kernel\syscon.c

# what to delete when cleaning
COPIEDSRC1 = \
  fatfs.c \
  fatdir.c \
  fattab.c \
  dosfns.c \
  error.c \
  prf.c \
  misc.c \
  syspack.c
COPIEDSRC2 = \
  task.c \
  memmgr.c \
  strings.c \
  blockio.c \
  chario.c \
  dsk.c \
  dosnames.c \
  syscon.c

#		*Implicit Rules*
.c.obj:
  $(CC) $(CFLAGS) -c {$< }

.cpp.obj:
  $(CC) $(CFLAGS) -c {$< }

#		*List Macros*


EXE_dependencies =  \
  ipl.obj \
  main.obj \
  blockio.obj \
  chario.obj \
  dsk.obj \
  error.obj \
  fatfs.obj \
  fatdir.obj \
  fattab.obj \
  task.obj \
  memmgr.obj \
  misc.obj \
  syspack.obj \
  dosnames.obj \
  prf.obj \
  strings.obj \
  syscon.obj \
  execrh.obj \
  procsupt.obj

HDRS = \
  globals.h \
  ../../hdr/device.h \
  ../../hdr/mcb.h \
  ../../hdr/pcb.h \
  ../../hdr/date.h \
  ../../hdr/time.h \
  ../../hdr/dosnames.h \
  ../../hdr/fat.h \
  ../../hdr/fcb.h \
  ../../hdr/process.h \
  ../../hdr/dcb.h \
  ../../hdr/sft.h \
  ../../hdr/exe.h

#		*Explicit Rules*
all:		production

production:	populate ipl.exe
		exe2bin ipl.exe ipl.sys <iplstart
		copy ipl.sys ..\..\dist
		del *.obj
		del ipl.sys
		del ipl.exe

populate:	$(FSSRC) $(SUPTSRC) $(IOSRC)
		release $(RELEASE)
		..\utils\pop $(FSSRC)
		..\utils\pop $(SUPTSRC)
		..\utils\pop $(IOSRC)

clobber:	clean
		del ipl.sys

clean:
		..\utils\rmfiles $(COPIEDSRC1)
		..\utils\rmfiles $(COPIEDSRC2)
		del ipl.exe
		del *.obj
		del *.bak
		del *.crf
		del *.xrf
		del *.map
		del *.las
		del *.lst

ipl.exe: populate ipl.cfg $(EXE_dependencies)
  $(LINK) /v/m/c/l/P-/LC:\BORLANDC\LIB @&&|
ipl.obj+
main.obj+
blockio.obj+
chario.obj+
dsk.obj+
error.obj+
fatfs.obj+
fatdir.obj+
fattab.obj+
task.obj+
memmgr.obj+
misc.obj+
syspack.obj+
dosnames.obj+
prf.obj+
strings.obj+
syscon.obj+
execrh.obj+
procsupt.obj
ipl
ipl		# map for debugging
$(LIBS)
|

#		*Individual File Dependencies*
main.obj:	main.c $(HDRS)

memmgr.obj:	memmgr.c $(HDRS)

misc.obj:	misc.c $(HDRS) 
		$(CC) $(CFLAGS) -c misc.c

syspack.obj:	syspack.c $(HDRS) 
		$(CC) $(CFLAGS) -c syspack.c

blockio.obj:	blockio.c $(HDRS) 
		$(CC) $(CFLAGS) -c blockio.c

chario.obj:	chario.c  $(HDRS)
		$(CC) $(CFLAGS) -c chario.c

dsk.obj:	dsk.c  $(HDRS)
		$(CC) $(CFLAGS) -c dsk.c

error.obj:	error.c $(HDRS)
		$(CC) $(CFLAGS) -c error.c

fatfs.obj:	fatfs.c $(HDRS)
		$(CC) $(CFLAGS) -c fatfs.c

fatdir.obj:	fatdir.c $(HDRS)
		$(CC) $(CFLAGS) -c fatdir.c

fattab.obj:	fattab.c $(HDRS)
		$(CC) $(CFLAGS) -c fattab.c

task.obj:	task.c $(HDRS)
		$(CC) $(CFLAGS) -c task.c

dosnames.obj:	dosnames.c $(HDRS) 
		$(CC) $(CFLAGS) -c dosnames.c

prf.obj:	prf.c $(HDRS)
		$(CC) $(CFLAGS) -c prf.c

strings.obj:	strings.c $(HDRS)
		$(CC) $(CFLAGS) -c strings.c

syscon.obj:	syscon.c $(HDRS)
		$(CC) $(CFLAGS) -c syscon.c

ipl.obj:	ipl.asm
		$(ASM) /mx/zi/DSTANDALONE/jMASM51 ipl;

#		*Compiler Configuration File*
ipl.cfg: ipl.mak
  copy &&|
$(CFLAGS)
| ipl.cfg


