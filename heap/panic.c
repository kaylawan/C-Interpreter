#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "panic.h"

void panic(const char* fmt, ...) {
   va_list args;
   va_start(args,fmt);
   vprintf(fmt,args);
   exit(1);
}
