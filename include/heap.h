#ifndef LIB_HEAP_H
    #define LIB_HEAP_H
#ifdef __cplusplus
    extern "C" {
    #endif
    #include <stddef.h>
    typedef struct heap_t heap_t;
    typedef struct {
        heap_t* hp;
        size_t use;
        size_t free;
        void* start;
        void* free_addr;
        size_t id;
    } heap_info_t;
    heap_t* heap_new(void);
    void* heap_malloc(heap_t* h,size_t size);
    void heap_free(heap_t* h,void* obj);
    void* heap_calloc(heap_t* h,size_t nmemb,size_t size);
    void* heap_realloc(heap_t* h,void* o,size_t news);
    void heap_destroy(heap_t** h);
    size_t heap_block_size(void* o);
    void heap_stat(heap_t* h,heap_info_t* inf);
    #ifdef __cplusplus
    }
    #endif
#endif 