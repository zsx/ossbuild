
The OpenJPEG makefile is not compatible with MSys/MinGW, so the following version (that is) was found:

http://lists.xcf.berkeley.edu/lists/gimp-developer/2008-October/021064.html


It's been modified for the directory it lives in + -Wl,--kill-at was added as a linker option.


Makefile.linux attempts to reassign the owner to root. This makefile corrects it.


On OSX, the patches from MacPorts were applied. Also, owned and group reassignement were removed in the makefile for osx.