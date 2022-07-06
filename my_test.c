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
    heap_free(heap, a);

    free(heap);
    free(region);
}
