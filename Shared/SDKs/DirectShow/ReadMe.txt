The DirectShow base classes were pulled from the DirectX SDK. To get the .lib files, you have to compile the base classes in Visual Studio 2008+ or by using the SDK commandline tools.

See the following blog post for more information:

http://blogs.msdn.com/karinm/archive/2008/04/25/how-to-in-search-of-strmbase-lib.aspx


Please make sure you build the Release_MBCS configuration!

Then copy strmbase.lib (NOT strmbasd.lib - the debug version) and all the headers to the lib/ and include/ folders (respectively).

