
wdk_msvcrt_winxp.obj: 	Pulled from Windows Driver Kit
wdk_msvcrt_win2000.obj: Pulled from Windows Driver Kit
wdk_oldnames.lib:	This is an oldnames.lib I pulled from the WDK but it 
			does not properly link with MSVC.

msvcrt.lib: 		Pulled from Windows Driver Kit

oldnames.lib:		Pulled from MSys/MinGW. The WDDK version does not allow
			you to correctly link! Make SURE you pull from MSys. 
			It's original filename is "libmoldname.a"

msvcrt_compat.lib: 	Contains export definitions that prevented proper linking.
			See ../../msvcrt-compat.h, ../../msvcrt-compat.def, and 
			../../msvcrt-compat.bat for details.

msvcrt_upgrade.obj: 	Just a copy of (wdk_)msvcrt_winxp.obj that provides a 
			way to use just one MSVC property file to reference 
			both Win32 and x64 platforms.


The need for msvcrt_upgrade.obj is documented here: 
http://kobyk.wordpress.com/2007/07/20/dynamically-linking-with-msvcrtdll-using-visual-c-2005/