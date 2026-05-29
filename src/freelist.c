
#include "freelist.h"
#include <stddef.h>
#include <string.h>
extern void* get(void);
extern void free_all(void);
static node_t* root = NULL;
static node_t* last = NULL;
static size_t nodes = 0;
void __free_node(node_t* n) {
    node_t* prev = n->prev;
    node_t* next = n->next;
    if (prev) prev->next = next;
    else root = next;
    if (next) next->prev = prev;
    else last = prev;
    nodes--;
    if (!nodes) free_all();
}
static node_t* merge(node_t* a, node_t* b) {
    if ((uint8_t*)a->mem + a->size != (uint8_t*)b->mem) return NULL;
    if (a->heapId != b->heapId) return NULL;
    a->size += b->size;
    __free_node(b);
    return a;
}
void append(void* ptr, size_t size, size_t heapId) {
    if (!root) {
        root = get();
        if (!root) return;
        root->heapId = heapId;
        root->mem = ptr;
        root->size = size;
        last = root;
        return;
    }
    node_t* new = get();
    if (!new) return;
    new->heapId = heapId;
    new->mem = ptr;
    new->size = size;
    last->next = new;
    new->prev = last;
    node_t* n = merge(last,new);
    if (n) last = n;
}
node_t* find_node(size_t s) {
    node_t* best = NULL;
    node_t* n = root;
    while (n) {
        if (n->size >= s) {
            if (!best || n->size < best->size)
                best = n;
        }
        n = n->next;
    }
    return best;
}
