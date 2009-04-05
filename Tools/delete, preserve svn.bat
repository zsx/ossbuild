@echo off

echo Are you sure? (y/n) 
set /p Sure=

if not "%Sure%"=="y" goto end

set OLDDIR=%CD%
set MYDIR=%~dp0
set BATCHFILE=%~nx0

cd /d %MYDIR%

rem Create temporary file for use with findstr (since it can only search actual files)

:TEMPLOOP
set /a MYTEMP=%RANDOM%+100000  
set MYTEMP=%TEMP%\Temp%MYTEMP:~-5%.TMP  
if exist %MYTEMP% goto TEMPLOOP

rem We create a temporary file and put the full path to the file inside of it and then 
rem use findstr to see if we should exclude it. Not very efficient, but it works.

FOR /F "tokens=*" %%G IN ('DIR /B /A-D /S *') DO (
	echo %%G > %MYTEMP%
	findstr /L /I /C:".svn" /C:"%BATCHFILE%" %MYTEMP% > NUL
	if errorlevel 1 (
		echo Deleting %%G
		del /F /Q "%%G"
	)
)

del %MYTEMP%

cd /d %OLDDIR% &rem restore current directory

@echo on

:end