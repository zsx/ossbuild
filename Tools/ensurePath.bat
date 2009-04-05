@echo off

set FILE_DIR=%~dp1

if "%FILE_DIR%" == "" goto done

:checkPath
if not exist "%FILE_DIR%" mkdir "%FILE_DIR%"
goto done

:done

REM Ensures path exists before running sed
REM setlocal enableextensions
REM for /f "tokens=*" %%a in ( 'call dir.bat "[$CustomOutputPath]"' ) do (set mydir=%%a)
REM if not exist "%mydir%" mkdir "%mydir%"
REM endlocal