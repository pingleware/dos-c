#
# $Header$
#
# $Log$
#

.AUTODEPEND

#		*Translator Definitions*
CC		= bcc +COMMAND.CFG
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
 ver.obj \
 prompt.obj \
 call.obj \
 echo.obj \
 for.obj \
 goto.obj \
 if.obj \
 label.obj \
 pause.obj \
 rem.obj \
 set.obj \
 shift.obj \
 cmd.obj \
 copy.obj \
 type.obj \
 del.obj \
 verify.obj \
 date.obj \
 time.obj \
 break.obj \
 batch.obj \
 dir.obj \
 cd.obj \
 mkdir.obj \
 ren.obj \
 rmdir.obj \
 prf.obj \
 env.obj \
 err.obj \
 path.obj \
 num.obj \
 strings.obj \
 scan.obj \
 dosopt.obj \
 home.obj \
 doslib.obj

#		*Explicit Rules*
all:		production

production:	command.exe
		exe2bin command.exe command.com
		copy command.com ..\..\dist
		copy autoexec.bat ..\..\dist
		del *.obj
		del command.exe
		del command.com

clobber:	clean
		del command.exe
		del command.com

clean:
		del *.obj
		del *.bak
		del *.crf
		del *.xrf
		del *.map
		del *.lst
		del *.las

command.exe: command.cfg $(EXE_dependencies)
  $(LINK) /v/x/c/P-/L$(LIBPATH) @&&|
c0t.obj+
ver.obj+
prompt.obj+
call.obj+
echo.obj+
for.obj+
goto.obj+
if.obj+
label.obj+
pause.obj+
rem.obj+
set.obj+
shift.obj+
cmd.obj+
copy.obj+
type.obj+
del.obj+
verify.obj+
date.obj+
time.obj+
break.obj+
batch.obj+
dir.obj+
cd.obj+
mkdir.obj+
ren.obj+
rmdir.obj+
prf.obj+
env.obj+
err.obj+
path.obj+
num.obj+
strings.obj+
scan.obj+
dosopt.obj+
home.obj+
doslib.obj
command
		# no map file
emu.lib+
maths.lib+
cs.lib
|


#		*Individual File Dependencies*
ver.obj: command.cfg ver.c 

prompt.obj: command.cfg prompt.c 

call.obj: command.cfg call.c 

echo.obj: command.cfg echo.c 

for.obj: command.cfg for.c 

goto.obj: command.cfg goto.c 

if.obj: command.cfg if.c 

label.obj: command.cfg label.c 

pause.obj: command.cfg pause.c 

rem.obj: command.cfg rem.c 

set.obj: command.cfg set.c 

shift.obj: command.cfg shift.c 

cmd.obj: command.cfg cmd.c 

copy.obj: command.cfg copy.c 

type.obj: command.cfg type.c 

del.obj: command.cfg del.c 

verify.obj: command.cfg verify.c 

date.obj: command.cfg date.c 

time.obj: command.cfg time.c 

break.obj: command.cfg break.c 

batch.obj: command.cfg batch.c 

dir.obj: command.cfg dir.c 

cd.obj: command.cfg cd.c 

mkdir.obj: command.cfg mkdir.c 

ren.obj: command.cfg ren.c 

rmdir.obj: command.cfg rmdir.c 

prf.obj: command.cfg ..\misc\prf.c 
	$(CC) -c ..\misc\prf.c

env.obj: command.cfg env.c 

err.obj: command.cfg err.c 

path.obj: command.cfg path.c 

num.obj: command.cfg ..\misc\num.c 
	$(CC) -c ..\misc\num.c

strings.obj: command.cfg ..\misc\strings.c 
	$(CC) -c ..\misc\strings.c

scan.obj: command.cfg scan.c 

dosopt.obj: command.cfg ..\misc\dosopt.c 
	$(CC) -c ..\misc\dosopt.c

home.obj: command.cfg ..\misc\home.c 
	$(CC) -c ..\misc\home.c

doslib.obj: command.cfg doslib.c 

#		*Compiler Configuration File*
command.cfg: command.mak
  copy &&|
-mt
-v
-vi-
-weas
-wpre
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DI86=1;PROTO=1;DEBUG=1
-P-.C
| command.cfg


