.AUTODEPEND

#		*Translator Definitions*
CC		= bcc +SYS.CFG
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

LIB	=  ..\..\lib\device.lib

EXE_dependencies =  \
 sys.obj \
 $(LIB)

#		*Explicit Rules*
production:	sys.exe
		tdstrip sys.exe
		copy sys.exe ..\..\dist
		del sys.obj
		del sys.exe

sys.exe:	sys.cfg $(EXE_dependencies)
		$(LINK) /v/x/c/P-/L$(LIBPATH) @&&|
c0s.obj+
sys.obj
sys
		# no map file
$(LIB)+
emu.lib+
maths.lib+
cs.lib
|


clobber:	clean
		del sys.exe

clean:
		del *.obj
		del *.bak
		del *.crf
		del *.xrf
		del *.map
		del *.lst
		del *.las

#		*Individual File Dependencies*
sys.obj: sys.cfg sys.c 

#		*Compiler Configuration File*
sys.cfg: sys.mak
  copy &&|
-v
-vi-
-wpro
-weas
-wpre
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DI86=1;PROTO=1
| sys.cfg


