Patches for ffmpeg on MaxOSX (Darwin):

*libswscale/swscale.h, libswscale/Makefile and libavfilter/avfilter.h patched with the latest 
patches from MacPorts for ffmpeg 0.5
* libavutil/internal.h has a little hack I added in order to solve a compilation issue with the 
llrint, lrint, etc. functions.