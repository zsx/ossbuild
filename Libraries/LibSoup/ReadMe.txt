


Ideally, all lines that #include <unistd.h> (and sys/time.h) would be surrounded with:

#ifndef _MSC_VER
#include <unistd.h>
#endif


Instead, when compiling for MSVC, we use a blank unistd.h and sys/time.h header.

Also, msvc-extras.h/msvc-extras.c contains some valid UNIX functions that needed to be mapped to their Win32-equivalents.
