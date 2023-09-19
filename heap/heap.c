#include "heap.h"
#include "panic.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#define BLOCK_SIZE (sizeof(struct block))
#define ALIGNMENT 16
#define ALIGN(size) (((size)+(ALIGNMENT-1)) & ~(ALIGNMENT-1))

volatile long mCount = 0;
volatile long fCount = 0;

struct block{
    struct block *next;
    struct block *prev;
    uint64_t padding;
    uint32_t size;
    uint16_t padding2;
    uint8_t padding3;
    uint8_t free;
}block_t;

static struct block *first_block = NULL;
static struct block *last_block = NULL;

void* my_malloc(size_t bytes) {
    mCount+=1;
    if(bytes==0){
        return NULL;
    }
    if((int)(bytes)<0){
        return NULL;
    }
    if((int)(bytes)>=(int)heap_size){
        return NULL;
    }

    // ensure that bytes is a multiple of 16
    bytes = (bytes + 15) & ~15;

    // check if the heap has been initialized
    if (first_block == NULL) {
        // initialize the heap with a single free block
        first_block = (struct block *)the_heap;
        first_block->size = heap_size - BLOCK_SIZE;
        first_block->free = 1;
        first_block->next = NULL;
        first_block->prev = NULL;
        last_block = first_block;
    }
    // search for the first free block that is large enough
    struct block *current_block = first_block;
    while (current_block != NULL && !(current_block->free && current_block->size >= bytes)) {
        current_block = current_block->next;
    }
    //no free block is large enough
    if (current_block == NULL) {
        return NULL;
    }
    if (current_block->size > bytes + BLOCK_SIZE) {
        // split the block into an allocated block and a free block
        struct block *new_block = (struct block *)(((char *) current_block) + BLOCK_SIZE + bytes);
        new_block->size = current_block->size - BLOCK_SIZE - bytes;
        new_block->free = 1;
        new_block->next = current_block->next;
        new_block->prev = current_block;
        if (current_block->next != NULL) {
            current_block->next->prev = new_block;
        } 
        else 
        {
            last_block = new_block;
        }
        current_block->next = new_block;
        current_block->size = bytes;
    }
    current_block->free = 0;
    return ((void *) current_block) + BLOCK_SIZE;
    //return (void*)(((uintptr_t)current_block + BLOCK_SIZE + 15) & ~15);
}

void my_free(void *ptr) {
    fCount+=1;
    // check if ptr is NULL
    if (ptr == NULL) {
        return;
    }
    //find the block corresponding to ptr.
    //since malloc returns the address of the actual memory block,
    //we subtract the block-size from this pointer to also free the metadata block
    //struct block *current_block = (struct block *)(ptr-BLOCK_SIZE);
    struct block *current_block = (struct block *)((char *)ptr - BLOCK_SIZE);
    // mark the block as free
    current_block->free = 1;
    // merge with adjacent free blocks
    if (current_block->prev != NULL && current_block->prev->free) {
        // merge with previous block
        current_block->prev->size += BLOCK_SIZE + current_block->size;
        current_block->prev->next = current_block->next;
        if (current_block->next != NULL) {
            current_block->next->prev = current_block->prev;
        } 
        else {
            last_block = current_block->prev;
        }
        current_block = current_block->prev;
    }
    
    if (current_block->next != NULL && current_block->next->free) {
        // merge with next block
        current_block->size += BLOCK_SIZE + current_block->next->size;
        current_block->next = current_block->next->next;
        if (current_block->next != NULL) {
            current_block->next->prev = current_block;
        } else {
            last_block = current_block;
        }
    }
}
