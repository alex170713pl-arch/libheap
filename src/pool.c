#include <stdlib.h>
#include <sys/mman.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
static size_t s = 40;
typedef struct _bk{
    struct _bk* next;
    void* addr;
    uint8_t* next_free;
    uint8_t* end;
} block_t;
static block_t first = {0};
static block_t* curr = &first;
static void __init(block_t* b) {
    void* mm = mmap(NULL,4096*1024,
    PROT_READ | PROT_WRITE,
    MAP_ANON | MAP_PRIVATE,-1,0);
    if (mm == MAP_FAILED) return;
    b->addr = mm;
    b->end = (uint8_t*)b->addr + 4096*1024;
    b->next_free = (uint8_t*)b->addr;
}
static void __select_bk(void) {
    if (!curr->addr) {
        __init(curr);
        return; 
    }
    uint8_t* p = curr->next_free;
    p += s;
    if (p > curr->end) {
        block_t* next = calloc(1,sizeof(block_t));
        if(!next) return;
        __init(next);
        if (!next->addr) {
            free(next);
            return;
        }
        curr->next = next;
        curr = next;
    }
}
void* get(void) {
    __select_bk();
    void* dat = curr->next_free;
    curr->next_free += s;
    return dat;
}
void free_all(void) {
    block_t* b = &first;
    while (b) {
        block_t* next = b->next;
        munmap(b->addr, 4096*1024);
        if (b != &first)
            free(b);
        b = next;
    }
    memset(&first,0,sizeof(block_t));
    curr = &first;
}