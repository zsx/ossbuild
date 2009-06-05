dnl This macros is intended to define platform specific tuned CFLAGS/CCASFLAGS
AC_DEFUN([AG_GST_CPU_TUNE],
[
  CPU_TUNE_CFLAGS=""
  CPU_TUNE_CCASFLAGS=""
  
  dnl tune build for Nokia N800   
  AC_ARG_ENABLE(cpu-tune-n800,
    AC_HELP_STRING([--enable-cpu-tune-n800], 
      [enable CFLAGS/CCASFLAGS tuned for Nokia N800]),
    [TUNE=yes],
    [TUNE=no]) dnl Default value
     
  if test "x$TUNE" = xyes; then
    AC_MSG_NOTICE(Build will be tuned for Nokia N800)
    AS_COMPILER_FLAG(-march=armv6j, 
      CPU_TUNE_CFLAGS="$CPU_TUNE_CFLAGS -march=armv6j")
    AS_COMPILER_FLAG(-mtune=arm1136j-s, 
      CPU_TUNE_CFLAGS="$CPU_TUNE_CFLAGS -mtune=arm1136j-s")
    dnl Some assembly code requires -fomit-frame-pointer
    AS_COMPILER_FLAG(-fomit-frame-pointer, 
      CPU_TUNE_CFLAGS="$CPU_TUNE_CFLAGS -fomit-frame-pointer")
    CPU_TUNE_CCASFLAGS="$CPU_TUNE_CCASFLAGS $CPU_TUNE_CFLAGS"
  fi
  
  dnl tune build using softfp
  AC_ARG_ENABLE(cpu-tune-softfp,
    AC_HELP_STRING([--enable-cpu-tune-softfp], 
      [enable build with softfp and vfp]),
    [TUNE=yes],
    [TUNE=no]) dnl Default value
     
  if test "x$TUNE" = xyes; then
    AS_COMPILER_FLAG(-mfloat-abi=softfp, 
      CPU_TUNE_CFLAGS="$CPU_TUNE_CFLAGS -mfloat-abi=softfp")
    AS_COMPILER_FLAG(-mfpu=vfp, 
      CPU_TUNE_CFLAGS="$CPU_TUNE_CFLAGS -mfpu=vfp")
         
    CPU_TUNE_CCASFLAGS="$CPU_TUNE_CCASFLAGS $CPU_TUNE_CFLAGS"
  fi
  
  dnl tune build using arm/thumb
  AC_ARG_ENABLE(cpu-tune-thumb,
    AC_HELP_STRING([--enable-cpu-tune-thumb], 
      [enable generation of thumb code for arm devices]),
    [TUNE=yes],
    [TUNE=no]) dnl Default value
     
  dnl tune build on Solaris with Sun Forte CC
  AS_COMPILER_FLAG([-xO5],
    CPU_TUNE_CFLAGS="$CPU_TUNE_CFLAGS -xO5")

  if test "x$TUNE" = xyes; then
    AS_COMPILER_FLAG(-mthumb, 
      CPU_TUNE_CFLAGS="$CPU_TUNE_CFLAGS -mthumb")
  fi
    
  AC_SUBST(CPU_TUNE_CFLAGS)
  AC_SUBST(CPU_TUNE_CCASFLAGS)
  
  if test "x$CPU_TUNE_CFLAGS" != "x"; then  
    AC_MSG_NOTICE(CPU_TUNE_CFLAGS   : $CPU_TUNE_CFLAGS)
    AC_MSG_NOTICE(CPU_TUNE_CCASFLAGS: $CPU_TUNE_CCASFLAGS)
  fi  

])

