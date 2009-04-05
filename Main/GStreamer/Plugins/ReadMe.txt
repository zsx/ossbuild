
ext/ is for things that require external libraries (other than the gst libraries, glib, and libxml), gst/ is for things that don't, gst-libs is for additional libraries (i.e. adding actual API)



everything outside of sys/ directories should be buildable on linux and windows (though building it on windows may be impractically difficult, depending on your needs). sys/ is for OS-specific things; generally it's obvious which OSes they should build on

For list of plugins that can be built (in Windows), go to:

http://gstreamer.freedesktop.org/wiki/BuildGStreamerWithMinGWAndMsys

Select good, ugly, bad, base, etc. and scroll all the way to the bottom for a list of plugins that can't be built in Windows.