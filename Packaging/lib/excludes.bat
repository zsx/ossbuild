if "%1" == "" goto DEFAULT
if not "%1" == "" goto CUSTOM

:CUSTOM
del /F /Q "%EXCLUDESFILE%"
:loop
if "%1" == "" goto END
echo %1 >> "%EXCLUDESFILE%"
shift
goto loop

:DEFAULT
echo \.svn\ > "%EXCLUDESFILE%"
echo \bin\ >> "%EXCLUDESFILE%"
echo \obj\ >> "%EXCLUDESFILE%"
echo .csproj.user >> "%EXCLUDESFILE%"
echo .suo >> "%EXCLUDESFILE%"
echo sign.bat >> "%EXCLUDESFILE%"
goto END

:END