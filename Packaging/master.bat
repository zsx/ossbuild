@echo off

call lib\env.bat

del /Q "%DEPLOYMENTDIR%\*.zip"

call "libraries.bat"

@echo on