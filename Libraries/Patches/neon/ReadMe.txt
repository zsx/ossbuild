
To get this to compile under msys/mingw, you have to change ws2tcpip.h (e.g. C:\msys\include\ws2tcpip.h), line 272 to be:

#ifndef socklen_t
typedef int socklen_t;
#endif