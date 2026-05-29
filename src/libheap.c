#include "freelist.h"
#include "../include/heap.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#define  HEAP_BLOCK_SIZE 2048*4096
#define PANIC(msg,...) \
    fprintf(stderr,msg,##__VA_ARGS__); \
    abort()

typedef struct {
    size_t size;
    size_t heapId;
} header_t;
typedef struct block {
    uint8_t* addr;
    uint8_t* max;
    struct block* next;
} block_t;
struct heap_t {
    block_t start;
    block_t* curr;
    size_t id;
};
static size_t id = 1;
static size_t __round(size_t s) {
    return ((s + 15) & ~15);
}
static void __make_header(void* p,size_t s,size_t id) {
    header_t h = {s,id};
    memcpy(p,&h, sizeof(h));
}
static void* __cut_node(node_t* n,size_t s) {
    void* d = n->mem;
    if (n->size == s) {
        __free_node(n);
    } else {
        n->mem += s;
        n->size -= s;
    }
    return d;
}
static _Bool __init_block(block_t* b,block_t* par) {
    b->addr = mmap(NULL,HEAP_BLOCK_SIZE,
    PROT_READ | PROT_WRITE,
    MAP_ANON | MAP_PRIVATE,-1,0);
    if (b->addr == MAP_FAILED) {
        return 0;
    }
    if (par) par->next = b;
    b->max = b->addr + HEAP_BLOCK_SIZE;
    return 1;
}
static block_t* __mk_block(block_t* par) {
    block_t* b = calloc(1,sizeof(block_t));
    if (!b) return NULL;
    if (!__init_block(b,par)) {
        free(b);
        return NULL;
    }
    return b;
}
static _Bool __select_block(heap_t* h,size_t s) {
    uint8_t* p = h->curr->addr;
    p +=s;
    if (p > h->curr->max) {
        block_t* b = __mk_block(h->curr);
        if (!b) return 0;
        __init_block(b,h->curr);
        h->curr = b;
    }
    return 1;
}
static void* __get_mem(block_t* b,size_t s) {
    void* d = b->addr;
    b->addr += s;
    return d;
}
heap_t* heap_new(void) {
    heap_t* h = calloc(1,sizeof(heap_t));
    if (!h) return NULL;
    if (!__init_block(&h->start,NULL)) {
        free(h);
        return NULL;
    }
    h->curr = &h->start;
    h->id = id++;
    return h;
}
void* heap_malloc(heap_t* h,size_t size) {
    if (!h) return NULL;
    if (!size) return NULL;
    void* data;
    size = __round(size);
    size += sizeof(header_t);
    node_t* n = find_node(size);
    if (n) data = __cut_node(n, size);
    else {
        if (!__select_block(h, size)) return NULL;
        data = __get_mem(h->curr, size);
    }
    __make_header(data,size - 16,h->id);
    return (uint8_t*)data + 16;
}
void heap_free(heap_t* h,void* obj) {
    header_t* head = (header_t*)((uint8_t*)obj - sizeof(header_t));
    if (head->heapId > id) PANIC("libheap: Corrupted pointer at %p\n",obj);
    if (head->size == 0) PANIC("libheap: Corrupted or already freed pointer at %p\n",obj);
    if (head->heapId != h->id) PANIC("libheap: Attempt to free pointer from heap with id %zu,got heap with id %zu\n",head->heapId,h->id);
    append(obj,head->size,head->heapId);
    memset(head,0,sizeof(header_t) + head->size);
}
void* heap_calloc(heap_t* h,size_t nmemb,size_t size) {
    if (!h) return NULL;
    size_t s = nmemb*size;
    if (s < size) return NULL;
    s = __round(s);
    void* p = heap_malloc(h,s);
    if (!p) return NULL;
    memset(p,0,s);
    return p;
}
void* heap_realloc(heap_t* h, void* o, size_t news) {
    if (!h || !o) return NULL;
    if (!news) {
        heap_free(h,o);
        return NULL;
    }
    void* newb = heap_malloc(h,news);
    if (!newb) return NULL;
    header_t* hed = (header_t*)((uint8_t*)o - sizeof(header_t));
    memcpy(newb,o,hed->size);
    heap_free(h,o);
    return newb;
}
void heap_destroy(heap_t** h) {
    if (!h || !(*h)) return;
    heap_t* hp = *h;
    munmap(hp->start.addr,HEAP_BLOCK_SIZE);
    block_t* s = hp->start.next;
    while (s) {
        munmap(s->addr,HEAP_BLOCK_SIZE);
        block_t* n = s->next;
        free(s);
        s = n;
    }
    free(hp);
    *h = NULL;
}
size_t heap_block_size(void* o) {
    if (!o) return 0;
    header_t* hd = (header_t*)((uint8_t*)o - sizeof(header_t));
    return hd->size;
}
void heap_stat(heap_t* h, heap_info_t* inf) {
    if (!h || !inf) return;
    inf->hp = h;
    inf->id = h->id;
    inf->start = h->start.addr;
    block_t* curr = h->curr;
    inf->free_addr = curr->addr;
    inf->free = curr->max - curr->addr;
    inf->use = 0;
    block_t* b = &h->start;
    while (b) {
        if (b == curr) {
            inf->use += (b->max - b->addr);
        } else if (b->addr) {
            inf->use += HEAP_BLOCK_SIZE;
        }
        b = b->next;
    }
}