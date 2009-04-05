rmdir /S /Q "%PKGDIR%"
del "%EXCLUDESFILE%"
cd /d %OLDDIR% &rem restore current directory
