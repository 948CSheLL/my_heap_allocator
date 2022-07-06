#include "include/heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    int i;

    /* 给堆分配空间，并对其进行初始化 */
    heap_t *heap = malloc(sizeof(heap_t));
    memset(heap, 0, sizeof(heap_t));
    /* Q: 这个region 目前还不知道有什么用 */
    void *region = malloc(HEAP_INIT_SIZE);
    memset(region, 0, HEAP_INIT_SIZE);
    
    /* 给堆中的每个空闲链表分配空间并初始化 */
    for (i = 0; i < BIN_COUNT; i++) {
        heap->bins[i] = malloc(sizeof(bin_t));
        memset(heap->bins[i], 0, sizeof(bin_t));
    }
    /* 第二个参数region 是一个指针类型，long 类型是64 位可以保存
     * 指针类型。
     */
    init_heap(heap, (long) region);
    
    printf("overhead = %d \n", overhead);

    void *a = heap_alloc(heap, 8);
    printf("a = %d size: 8 \n", (int) a);
    void *b = heap_alloc(heap, 128);
    printf("b = %d size: 128 \n", (int) b);
    void *c = heap_alloc(heap, 8);
    printf("c = %d size: 8 \n", (int) c);

    printf("\nfreeing b \n");
    heap_free(heap, b);

    void* d = heap_alloc(heap, 8);
    printf("d = %d size: 8 \n", (int) d);

    void* e = heap_alloc(heap, 16);
    printf("e = %d size: 16 \n", (int) e);
    
    void* f = heap_alloc(heap, 8);
    printf("f = %d size: 8 \n", (int) f);

    void* g = heap_alloc(heap, 8);
    printf("g = %d size: 8 \n", (int) g);

    printf("\nfreeing d & f \n");
    heap_free(heap, d);
    heap_free(heap, f);
    
    printf("\nfreeing e\n");
    heap_free(heap, e);

    void* h = heap_alloc(heap, 128);
    printf("h = %d size: 128 \n", (int) h);
    printf("\n");

    for (i = 1; i <= 2048; i += i) printf("size: %d -> bin: %d \n", i, get_bin_index(i));

    for (i = 0; i < BIN_COUNT; i++) {
        free(heap->bins[i]);
    }

    free(heap);
    free(region);
}
