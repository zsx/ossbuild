
Some parts of this were pulled from the Windows Driver Kit (WDK) and some from MSys/MinGW. Please see the lib/ folder ReadMe files for more information.

msvcrt-compat.h should be a forced include if you're doing file i/o.

msvcrt-compat.def is for generating lib files that properly define some missing function exports in msvcrt.

msvcrt-compat.bat is to generate .lib files from msvcrt-compat.def. You must have VS2008 or lib.exe in your path for this to work.