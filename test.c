#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "heap/heap.h"
#include "heap/panic.h"

#define HEAP_SIZE 8500

long heap_size = HEAP_SIZE;
long the_heap[HEAP_SIZE/sizeof(long)];

void recursively_malloc(int num);
void verify_values(int num, long *arr[5]);

int main() {
    /**
     * This test case recursively mallocs blocks of varying size.
     * During the free process, the test frees blocks from latest to oldest (presumably right to left)
     * but frees the sub-blocks from oldest to latest (right to left).
     * This tests for correct left and right merging.
     * 
     * The test also checks that all of the allocated bytses are 16-byte aligned
    */

    recursively_malloc(0);

    /**
     * Here are a few other edge case checks
    */
    int *null_ptr = malloc(0);
    if (null_ptr != NULL && (long)null_ptr % 16 != 0) {
        panic("This should return NULL or a valid pointer.");
    }

    int *negative_ptr = malloc(-5);
    if (negative_ptr != NULL) {
        panic("This should return NULL.");
    }

    panic("Passed\n");
}

void recursively_malloc(int num) {
    long *temp[5];
    for (int i = 1; i <= 5; i++) {
        temp[i - 1] = malloc(num * 5 + i);
        *(temp[i - 1]) = num * 5 + i;
    }

    if (num < 19) {
        recursively_malloc(num + 1);
    }
    verify_values(num, temp);

    for (int i = 0; i < 5; i++) free(temp[i]);

}

void verify_values(int num, long *arr[5]) {
    for (int i = 0; i < 5; i++) {
        long *ptr = arr[i];
        if (*ptr != num * 5 + i + 1) {
            panic("The values were not preserved after subsequent malloc calls.");
        }
        if ((long)ptr % 16 != 0) {
            panic("The returned pointers are not 16-byte aligned");
        }
    }
}