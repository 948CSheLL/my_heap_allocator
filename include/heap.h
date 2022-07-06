#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

#define HEAP_INIT_SIZE 0x10000
#define HEAP_MAX_SIZE 0xF0000
#define HEAP_MIN_SIZE 0x10000

#define MIN_ALLOC_SZ 4

#define MIN_WILDERNESS 0x2000
#define MAX_WILDERNESS 0x1000000

#define BIN_COUNT 9
#define BIN_MAX_IDX (BIN_COUNT - 1)

typedef unsigned int uint;

/* 定义的块的结构 */
typedef struct node_t {
    uint hole;              // 块是否空闲
    uint size;              // 块的大小
    struct node_t* next;
    struct node_t* prev;
} node_t;

/* 定义的页脚结构 */
typedef struct { 
    node_t *header;         // 指向页眉
} footer_t;

/* 定义的空闲链表结构 */
typedef struct {
    node_t* head;           // 指向第一个空闲块
} bin_t;

/* 定义的堆结构 */
typedef struct {
    long start;
    long end;
    bin_t *bins[BIN_COUNT];
} heap_t;

/* 页眉和页脚的大小之和 */
static uint overhead = sizeof(footer_t) + sizeof(node_t);

/* 初始化堆 */
void init_heap(heap_t *heap, long start);

/* 从堆中分配size 大小的空间 */
void *heap_alloc(heap_t *heap, size_t size);
/* 向堆中添加之前分配的空间 */
void heap_free(heap_t *heap, void *p);
/* 扩展整个堆，此函数没有实现 */
uint expand(heap_t *heap, size_t sz);
/* 缩小整个堆，此函数没有实现 */
void contract(heap_t *heap, size_t sz);

/* 根据大小选择对应的空闲链表下标 */
uint get_bin_index(size_t sz);
/* 给一个块创建一个页脚 */
void create_foot(node_t *head);
/* 获得一个指向页脚的指针 */
footer_t *get_foot(node_t *head);

node_t *get_wilderness(heap_t *heap);

#endif
