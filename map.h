#pragma once
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define SIZE 8192

//Slice Struct
typedef struct Slice{
    char const *start;
    size_t const length;
}Slice;

//creates a new Slice struct with desired starting index and size.
Slice createSlice(char const * const start, size_t const length){
    Slice slice = {start, length};
    return slice;
}

//Node Struct
typedef struct Node{
    struct Slice key;
    uint64_t value;
    struct Node *next;
}Node;

//constrctor for hashmap Node
struct Node createNode(struct Slice key, uint64_t value, struct Node *next){
    struct Node node = {key, value, next};
    return node;
}

//HashMap struct
typedef struct HashMap{
    int size;
    struct Node **list;
}HashMap;

//constructor for hashmap
HashMap* generateMap(int size){
    int len = size;
    HashMap* map = (struct HashMap* )malloc(sizeof(struct HashMap));
    map->size = len;
    map->list = (struct Node **)malloc(sizeof(struct Node *)* size);
    int index = 0;
    //this will initialize a new HashMap that is completely NULL.
    while(index< size){
        map->list[index] = NULL;
        index++;
    }
    return map;
};

//hasher function from original slice.h C++ code
int hasher(struct Slice key){
    size_t out = 5381;
    for(size_t i=0;i<key.length;i++){
        char const c = key.start[i];
        out = out * 33 + c;
    }
    return out%8192;
}

//checks if two slices are equal
bool sliceEquals(Slice slice1, Slice slice2){
    int length1 = slice1.length;
    int length2 = slice2.length;
    if(length1!=length2){
        return false;
    }
    else{
        for(size_t i=0; i<length1;i++){
            if(slice1.start[i]!= slice2.start[i]){
                return false;
            }
        }
        return true;
    }
}

void insert(Slice key, HashMap *map, uint64_t value){
    int position = hasher(key);
    struct Node *list = map->list[position]; //attempts to look up Node
    struct Node *temp = list;
    int len = map->size;
    while(temp!=NULL){
        if (sliceEquals(temp->key, key)){
            temp->value = value;
            return;
        }
        else {
            len+=1;
            temp = temp->next;
        }
    }
    struct Node *pointer = (struct Node*)malloc(sizeof(struct Node));
    struct Node newNode = createNode(key,value,list);
    //memcpy
    memcpy(pointer, &newNode, sizeof(struct Node));
    map->list[position] = pointer;
}

uint64_t search(HashMap *map, Slice key){
    int position = hasher(key);
    struct Node *Node = map->list[position];
    while(Node!=NULL){
        bool equals = sliceEquals(Node->key, key);
        if(equals==true){
            uint64_t myVal = Node->value;
            return myVal;
        }
        Node = Node->next;
    }
    return 0;
}