#include "include/heap.h"
#include "include/llist.h"
#include <stdio.h>

uint offset = 8;

void init_heap(heap_t* heap, long start)
{
    /* 所有指针都是指向一块内存的一个字节，然后根据指针的类型来
     * 对这个字节及其后面的字节进行解析。因此强制类型转换是没有
     * 问题的（void* -> long -> node_t）
     * 参开：https://blog.csdn.net/qq_41854911/article/details/124888794
     */
    node_t* init_region = (node_t*)start;
    /* hole 等于1 表示块是空闲的 */
    init_region->hole = 1;
    init_region->size
        = (HEAP_INIT_SIZE) - sizeof(node_t) - sizeof(footer_t);

    create_foot(init_region);
    /* 第一个是参数是找到的空闲链表，第二个参数传进函数的是上面
     * 申请的整个堆区域的起始地址
     */
    add_node(heap->bins[get_bin_index(init_region->size)], init_region);

    /* 保存页眉header 的起始地址，即整个堆的起始地址 */
    heap->start = (void*)start;
    /* 保存页脚footer 的起始地址 */
    heap->end = (void*)(start + HEAP_INIT_SIZE);
}

void* heap_alloc(heap_t* heap, size_t size)
{
    uint index = get_bin_index(size);
    bin_t* temp = (bin_t*)heap->bins[index];
    node_t* found = get_best_fit(temp, size);

    while (found == NULL) {
        if (index + 1 >= BIN_COUNT)
            return NULL;

        temp = heap->bins[++index];
        /* 如果下面的found 不为NULL，说明找到了一个块 */
        found = get_best_fit(temp, size);
    }
    /* overhead 的大小其实就是页眉和页脚的总和：
     * sizeof(footer_t) + sizeof(node_t)。
     * 下面的if 语句使用来判断找到的块是否能够被拆分。
     */
    if ((found->size - size) > (overhead + MIN_ALLOC_SZ)) {
        node_t* split
            = (node_t*)(((char*)found + sizeof(node_t) + sizeof(footer_t))
                + size);
        split->size
            = found->size - size - sizeof(node_t) - sizeof(footer_t);
        split->hole = 1;
        /* 给分割的部分添加页脚footer */
        create_foot(split);
        /* 获取split 应该存放的空闲链表的下标 */
        uint new_idx = get_bin_index(split->size);
        /* 将split 往空闲链表上添加 */
        add_node(heap->bins[new_idx], split);
        /* 更改需要使用的的块的大小为size */
        found->size = size;
        /* 由于更改之后found 没有页脚，因此需要添加一个页脚 */
        create_foot(found);
    }
    /* 标记found 被使用 */
    found->hole = 0;
    /* 将found 从相应的heap->bins[index] 中删除 */
    remove_node(heap->bins[index], found);

    node_t* wild = get_wilderness(heap);
    /* 说明此时需要扩充堆的容量 */
    if (wild->size < MIN_WILDERNESS) {
        uint success = expand(heap, 0x1000);
        if (success == 0) {
            return NULL;
        }
    /* 说明此时需要缩减堆的容量 */
    } else if (wild->size > MAX_WILDERNESS) {
        contract(heap, 0x1000);
    }

    found->prev = NULL;
    found->next = NULL;
    /* Q: 不懂为啥要返回found->next的地址 */
    return &found->next;
}

void heap_free(heap_t* heap, void* p)
{
    bin_t* list;
    footer_t *new_foot, *old_foot;

    /* offset = 8，是因为node_t 结构前两个成员是uint，两者大小
     * 总和是8
     */
    /* 由于heap_alloc 函数的返回值是node_t结构next成员的地址，所以
     * 要获取node_t 起始地址需要减去前面两个成员的offset 。
     */
    node_t* head = (node_t*)((char*)p - offset);
    /* 如果要释放的区域是heap 的起始位置 */
    printf("head = %lu\n", (unsigned long) head);
    printf("head = %lu\n", (unsigned long) heap->start);
    /* 感觉下面的if 语句有点儿多余，会产生小碎片，个人感觉去
     * 个人感觉去掉会更好
     */
    if (head == (node_t*)(uintptr_t)heap->start) {
        head->hole = 1;
        add_node(heap->bins[get_bin_index(head->size)], head);
        return;
    }
    /* 获取head 物理位置下一个块的地址 */
    node_t* next = (node_t*)((char*)get_foot(head) + sizeof(footer_t));
    footer_t* f = (footer_t*)((char*)head - sizeof(footer_t));
    /* 获取head 物理位置上一个块的地址 */
    node_t* prev = f->header;
    /* 如果物理位置上一个块空闲，那么可以合并 */
    if (prev->hole) {
        /* 先获取上一个块所在的空闲链表 */
        list = heap->bins[get_bin_index(prev->size)];
        /* 将prev 块先移除 */
        remove_node(list, prev);

        /* prev的大小应该是页眉、页脚和有效存储部分大小的综合 */
        prev->size += overhead + head->size;
        new_foot = get_foot(head);
        new_foot->header = prev;

        head = prev;
    }

    /* 如果物理位置下一个块空闲，那么可以合并 */
    if (next->hole) {
        /* 下面的合并操作和上面的类似 */
        list = heap->bins[get_bin_index(next->size)];
        remove_node(list, next);

        head->size += overhead + next->size;

        old_foot = get_foot(next);
        old_foot->header = 0;
        next->size = 0;
        next->hole = 0;

        new_foot = get_foot(head);
        new_foot->header = head;
    }

    head->hole = 1;
    add_node(heap->bins[get_bin_index(head->size)], head);
}
/* 作者没有实现 */
uint expand(heap_t* heap, size_t sz) { return 0; }

/* 作者没有实现 */
void contract(heap_t* heap, size_t sz) { return; }

uint get_bin_index(size_t sz)
{
    uint index = 0;
    /* 这个策略意思是当sz 小于4，index=0，大于等于4 的数需要获得其
     * 位数减1，index 的值就是这个值减2，不过index 有一定的范围，
     * 用BIN_MAX_IDX 符号常量进行规范，这个策略可能会造成一种现象，
     * index 越大，其可以映射的sz 值越多。
     */
    sz = sz < 4 ? 4 : sz;
    
    while (sz >>= 1)
        index++;
    index -= 2;

    if (index > BIN_MAX_IDX)
        index = BIN_MAX_IDX;
    return index;
}

void create_foot(node_t* head)
{
    footer_t* foot = get_foot(head);
    foot->header = head;
}

footer_t* get_foot(node_t* node)
{
    return (footer_t*)((char*)node + sizeof(node_t) + node->size);
}

node_t* get_wilderness(heap_t* heap)
{
    /* Q: 为什么不直接返回(node_t *) heap->start */
    footer_t* wild_foot = (footer_t*)((char*)heap->end - sizeof(footer_t));
    return wild_foot->header;
}
