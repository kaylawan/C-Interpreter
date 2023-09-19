#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "heap/heap.h"
#include "heap/panic.h"

#define HEAP_SIZE 100000

long heap_size = HEAP_SIZE;
long the_heap[HEAP_SIZE/sizeof(long)];

int main(){
    long m1 = mCount;
    long f1 = fCount;
    // test 1: malloc test
    uint64_t blocks[100];
    for(int i = 0; i < 100; i++) {
        blocks[i] = (uint64_t)(malloc(8));
        if (blocks[i] == 0) {
            printf("test 1 fail at block: %i\n", (i + 1));
            return 0;
        }
        m1++;
    }
    puts("malloc test pass");

    // free test
    for(int i = 99; i >= 0; i--) {
        free((void*)blocks[i]);
        f1++;
    }
    puts("free test pass");

    //tests that malloc/free counts are still imcemented even when they return NULL
    void *mallocZero = malloc(0);
    m1++;
    void * mallocTooBig=malloc(892737828);
    m1++;
    if(m1!=mCount){
        printf("count doesn't match \n");
    }
    else{
        printf("count match \n");
    }
    if(mallocZero != NULL || mallocTooBig != NULL){
        printf("malloc wrong \n");
    } else{
        printf("malloc param test pass \n");
    }

    for(int i=0;i<100;i++){
        void * pointer = malloc(16);
        if(pointer==NULL){
            printf("unable to malloc free space \n");
        }
        else{
            free(pointer);
        }
    }
    printf("malloc and free test pass \n");
}