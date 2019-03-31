#
# $Header$
#
# $Log$
#

.AUTODEPEND

#		*Translator Definitions*
CC		= bcc +HELP.CFG
ASM		= tasm
LIB		= tlib
LINK		= tlink
LIBPATH		= C:\BORLANDC\LIB
INCLUDEPATH	= C:\BORLANDC\INCLUDE


#		*Implicit Rules*
.c.obj:
  $(CC) -c {$< }

.cpp.obj:
  $(CC) -c {$< }

#		*List Macros*


EXE_dependencies =  \
 helpexe.obj

#		*Explicit Rules*
all:		production

production:	help.exe
		tdstrip help
		copy help.exe ..\..\dist
		copy helpfile ..\..\dist
		del $(EXE_dependencies)
		del help.exe

help.exe:	help.cfg $(EXE_dependencies)
		$(LINK) /v/x/c/P-/L$(LIBPATH) @&&|
c0s.obj+
helpexe.obj
help
		# no map file
graphics.lib+
emu.lib+
maths.lib+
cs.lib
|


#		*Individual File Dependencies*
helpexe.obj: help.cfg helpexe.c 

#		*Compiler Configuration File*
help.cfg: help.mak
  copy &&|
-v
-vi-
-weas
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DI86=1;PROTO=1;
| help.cfg


