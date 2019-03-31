rem echo off
rem
rem Build file for dos-c
rem

cd lib
call ..\src\utils\makelibm
cd ..

cd src\drivers
make -f device production
cd ..\..

cd src\boot
make -f boot production
cd ..\..

cd src\ipl
make -f ipl production
cd ..\..

cd src\kernel
make -f kernel production
cd ..\..

cd src\utils
make -f sys production
cd ..\..

cd src\command
make -f command production
cd ..\..

rem $Header:   C:/dos-c/build.bav   1.0   05 Jul 1995 18:44:06   patv  $
rem
rem $Log:   C:/dos-c/build.bav  $
rem  
rem     Rev 1.0   05 Jul 1995 18:44:06   patv
rem  Initial revision.
rem
