
This library's build system is a bit odd. To compile with msys/mingw using their default configure.js script, you'll need to apply libxml2-msys.patch.

The patch is provided courtesy of http://live.gnome.org/Planner/Windows

This patch has already been applied to the source. You'll want to re-apply it for subsequent versions.

But you only need it if you're not using the autotools configure/make system (which is what we use).