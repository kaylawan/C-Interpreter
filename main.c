// libc includes (available in both C and C++)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Implementation includes HashMap data structure (which includes Slice)
#include "map.h"

//make sure hashmap is big enough to support large operations/requests
#define SIZE 8192

typedef struct Interpreter{
    char const * const program;
    char const * current;
}Interpreter;

struct HashMap * map;
bool consume(const char *str, struct Interpreter *x);
uint64_t expression(bool effects, struct Interpreter *x);

void fail(struct Interpreter *x) {
    size_t offset = (size_t)(x->current-x->program);
    printf("failed at offset %ld\n", offset);
    printf("%s\n", x->current);
    exit(1);
}

void end_or_fail(struct Interpreter *x) {
    while (isspace(*(x->current))) {
      x->current += 1;
    }
    if (*x->current != 0) fail(x);
}
void consume_or_fail(const char* str, struct Interpreter *x) {
    if (!consume(str, x)) {
        fail(x);
    }
}

void skip(struct Interpreter *x) {
    while (isspace(*(x->current))) {
        x->current += 1;
    }
}

bool consume(const char* str, struct Interpreter *x) {
    skip(x);
    size_t i = 0;
    while(true){
      char const expected = str[i];
      char const found = x->current[i];
      if (expected == 0) {
        x->current += i;
        return true;
      }
      if (expected != found) {
        return false;
      }
      i += 1;
    }
}

typedef struct BoolIdentifier{
    Slice slice;
    bool valid; 
}BoolIdentifier;

struct BoolIdentifier consume_identifier(struct Interpreter *x){
    skip(x);
    if(isalpha(*x->current)) {
      char const * start = x->current;
      do {
        x->current += 1;
      } while(isalnum(*x->current));
      struct Slice slice = createSlice(start, (x->current-start));
      struct BoolIdentifier boolIdentifier = {slice, true};
      return boolIdentifier;
    } 
    else {
        char const * start = x->current;
        struct Slice slice = createSlice(start, (x->current-start));
        struct BoolIdentifier boolIdentifier= {slice, false};
        return boolIdentifier;
    }
}

//struct for consume_literal that determines true/false case. calls fail() in false case!
typedef struct literalBool{
    uint64_t value;
    bool valid;
}literalBool;

//true is valid, false is not valid
struct literalBool consume_literal(struct Interpreter *x) {
    skip(x);
    if (isdigit(*(x->current))) {
        uint64_t v = 0;
        do {
            v = 10*v + ((*(x->current)) - '0');
            x->current += 1;
        } 
        while(isdigit(*(x->current)));
        struct literalBool value = {v, true};
        return value;
    } 
    else {
        struct literalBool value = {0, false};
        return value;
    }
}

// The plan is to honor as many C operators as possible with
// the same precedence and associativity
// e<n> implements operators with precedence 'n' (smaller is higher)
// () [] . -> ...
uint64_t e1(bool effects, struct Interpreter *x) {
    struct BoolIdentifier identifier = consume_identifier(x);
    if (identifier.valid == true) {
        uint64_t v = search(map, identifier.slice);
        return v;
    } 
    else {
        struct literalBool v = consume_literal(x);
        if(v.valid==true){
            return v.value;
        }
        else if(consume("(", x)){
            uint64_t v = expression(effects, x);
            consume(")", x);
            return v;
        }
        else{
            fail(x);
        }
    }
    return 0; //this isn't supposed to be here
}

// ++ -- unary+ unary- ... (Right)
uint64_t e2(bool effects,  struct Interpreter *x) {
    return e1(effects, x);
}

// * / % (Left)
uint64_t e3(bool effects, struct Interpreter *x) {
    uint64_t v = e2(effects, x);
    while (true) {
        if (consume("*", x)) {
            v = v * e2(effects, x);
        } 
        else if (consume("/", x)) {
            uint64_t right = e2(effects, x);
            v = (right == 0) ? 0 : v / right;
        } 
        else if (consume("%", x)) {
            uint64_t right = e2(effects, x);
            v = (right == 0) ? 0 : v % right;
        } 
        else {
            return v;
        }
    }
}

// (Left) + -
uint64_t e4(bool effects, struct Interpreter *x) {
    uint64_t v = e3(effects, x);
    while (true) {
        if (consume("+", x)) {
            v = v + e3(effects, x);
        } else if (consume("-", x)) {
            v = v - e3(effects, x);
        } else {
            return v;
        }
    }
}

// << >>
uint64_t e5(bool effects, struct Interpreter *x) {
    return e4(effects, x);
}

// < <= > >=
uint64_t e6(bool effects, struct Interpreter *x) {
    return e5(effects, x);
}

// == !=
uint64_t e7(bool effects, struct Interpreter *x) {
    return e6(effects, x);
}

// (left) &
uint64_t e8(bool effects, struct Interpreter *x) {
    return e7(effects, x);
}

// ^
uint64_t e9(bool effects, struct Interpreter *x) {
    return e8(effects, x);
}

// |
uint64_t e10(bool effects, struct Interpreter *x) {
    return e9(effects, x);
}

// &&
uint64_t e11(bool effects, struct Interpreter *x) {
    return e10(effects, x);
}

// ||
uint64_t e12(bool effects, struct Interpreter *x) {
    return e11(effects, x);
}

// (right with special treatment for middle expression) ?:
uint64_t e13(bool effects, struct Interpreter *x) {
    return e12(effects, x);
}

// = += -= ...
uint64_t e14(bool effects, struct Interpreter *x) {
    return e13(effects, x);
}

// ,
uint64_t e15(bool effects, struct Interpreter *x) {
    return e14(effects, x);
}

uint64_t expression(bool effects, struct Interpreter *x) {
    return e15(effects, x);
}

bool statement(bool effects, struct Interpreter *x) {
    if (consume("print", x)) {
        // print ...
        uint64_t v = expression(effects, x);
        if (effects) {
            printf("%ld\n",v);
        }
        return true;
    } 
    else{
        struct BoolIdentifier identifier = consume_identifier(x);
        if(identifier.valid==true){
            if(consume("=", x)){
                uint64_t v = expression(effects, x);
                if(effects==true){
                    insert(identifier.slice, map, v);
                }
                return true;
            }
            else{
                fail(x);
            }
        }
    }
    return false;
}

void statements(bool effects, struct Interpreter *x) {
    while (statement(effects, x));
}

void run(struct Interpreter *x) {
    statements(true, x);
    end_or_fail(x);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr,"usage: %s <file name>\n",argv[0]);
        exit(1);
    }

    // open the file
    int fd = open(argv[1],O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    // determine its size (std::filesystem::get_size?)
    struct stat file_stats;
    int rc = fstat(fd,&file_stats);
    if (rc != 0) {
        perror("fstat");
        exit(1);
    }

    // map the file in my address space
    char const* prog = (char const *)mmap(
        0,
        file_stats.st_size,
        PROT_READ,
        MAP_PRIVATE,
        fd,
        0);
    if (prog == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    map = generateMap(SIZE);
    struct Interpreter x = {prog, prog};
    struct Interpreter *newX = &x;
    run(newX);
    return 0;
}

// vim: tabstop=4 shiftwidth=2 expandtab 