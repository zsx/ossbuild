
ofmt_stub.o in the lib/ folder comes from the source for mingw-runtime 3.15. It provides a stub for get_output_format.

It has not been compiled/tested for x64!


Compiled (in MSys/MinGW) with:

as ofmt_stub.s -o ofmt_stub.o