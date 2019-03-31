echo off
rem
rem Create a distribution floppy
rem
rem $Header$
rem $Log$
rem
echo This utility will create a distribution floppy on the disk in drive A:
pause
cd dist

rem try to transfer system files -- abort ifit cannot.
sys a:
if errorlevel 1 goto out

rem copy remaining files
echo System transferred -- copying remaining files
echo on
copy ..\src\misc\autoexec.bat a:
copy ..\src\misc\config.sys a:
copy boot.bin a:
copy sys.exe a:
copy help.exe a:
copy helpfile a:
copy boot1440.bin a:
copy boot720.bin a:
copy boot360.bin a:
copy ..\doc\*.txt a:
echo off
label a: dos-c_v0_91

rem exit methods
goto done
:out
echo Floppy creation aborted
:done
cd ..
