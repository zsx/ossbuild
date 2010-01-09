
Had to edit all lines that #include <unistd.h> and surrounded them with:

#ifndef _MSC_VER
#include <unistd.h>
#endif

For now, fontconfig has to be built by msys/mingw and then all public headers modified with the above #ifndef _MSC_VER statements. The dev files are placed in the Shared/ directory.