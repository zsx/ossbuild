
Modified synaescope.c in synaesthesia (starting from line 28):

#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#ifndef _MSC_VER  
  #include <sys/time.h>
  #include <time.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _MSC_VER
  #include <unistd.h>
#endif
#include <string.h>
#include <assert.h>

It was:

#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>