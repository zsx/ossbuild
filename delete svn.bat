@echo off

echo Are you sure? (y/n) 
set /p Sure=

if not "%Sure%"=="y" goto end

set OLDDIR=%CD%
set MYDIR=%~dp0

cd /d %MYDIR%

FOR /F "tokens=*" %%G IN ('DIR /B /AD /S *.svn*') DO RMDIR /S /Q "%%G"

cd /d %OLDDIR% &rem restore current directory

@echo on

:end