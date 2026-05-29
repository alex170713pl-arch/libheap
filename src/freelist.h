
#include <stddef.h>
#include <stdint.h>
typedef struct _node {
    uint8_t* mem;
    size_t size;
    size_t heapId;
    struct _node* next;
    struct _node* prev;
} node_t;
void __free_node(node_t* n);
void append(void* ptr, size_t size, size_t heapId);
node_t* find_node(size_t s);