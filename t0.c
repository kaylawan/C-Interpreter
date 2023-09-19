#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "heap/heap.h"
#include "heap/panic.h"

#define M 20
#define N 100000

#define HEAP_SIZE (1 << 20)

long heap_size = HEAP_SIZE;
long the_heap[HEAP_SIZE/sizeof(long)];

void* last[M] = {};

int main() {

    for (uint32_t i=0; i<M; i++) {
        last[i] = 0;
    }

    long m1 = mCount;
    long f1 = fCount;

    for (uint32_t i=0; i<N; i++) {
        uint32_t x = rand() % M;
        if (last[x] != 0) {
            free(last[x]);
            last[x] = 0;
        }
        size_t sz = (size_t) (rand() % 1000 + 1);
        last[x] = malloc(sz);
        if (last[x] == 0) {
            panic("*** failed to allocate %d\n",sz);
        }
        char* ptr = (char*) last[x];
        ptr[0] = 66;
        ptr[sz-1] = 77;
    }

    for (uint32_t i=0; i<M; i++) {
        if (last[i] != 0) {
            free(last[i]);
        }
    }

    long m2 = mCount - m1;
    long f2 = fCount - f1;
  
    if (m2 != f2) {
        printf("m2 %ld\n",m2);
        printf("f2 %ld\n",f2);
    } else {
        printf("count match\n");
    }

    if (m2 != N) {
        printf("*** wrong count %ld\n",m2);
    } else {
        printf("count ok\n");
    }

    return 0;
}
