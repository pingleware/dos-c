rem echo off
rem
rem Clean directories for dos-c distribution
rem

del *.bak
del *.las

cd lib
del *.lib
cd ..

cd src\boot
make -f boot clean
cd ..\..

cd src\drivers
make -f device clean
cd ..\..

cd src\ipl
make -f ipl clean
cd ..\..

cd src\kernel
make -f kernel clean
cd ..\..

cd src\utils
make -f sys clean
cd ..\..

cd src\command
make -f command clean
cd ..\..

rem $Header:   C:/dos-c/clean.bav   1.0   05 Jul 1995 18:44:06   patv  $
rem
rem $Log:   C:/dos-c/clean.bav  $
rem  
rem     Rev 1.0   05 Jul 1995 18:44:06   patv
rem  Initial revision.
rem
