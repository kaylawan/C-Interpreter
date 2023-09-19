#ifndef _HEAP_H_
#define _HEAP_H_

#include <stdlib.h>

#undef malloc
#define malloc(n) my_malloc(n)

#undef free
#define free(p) my_free(p)

// heap size in bytes, defined by the test
extern long heap_size;

// memory for the heap, defined by the test
// must contain heap_size bytes
extern long the_heap[];

// number of calls to malloc, defined and updated by the heap implementation
extern volatile long mCount;

// number of calls to free, defined and updated by the heap implementation
extern volatile long fCount;

extern void* my_malloc(size_t);
extern void my_free(void*p);

#endif
