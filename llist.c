#include "include/llist.h"

void add_node(bin_t *bin, node_t* node) {
    node->next = NULL;
    node->prev = NULL;

    node_t *temp = bin->head;

    /* 刚开始建立堆的时候，将heap->bins 都初始化成0 了
     * init_heap 函数调用此函数时，一定会走下面这个if，
     * 此时传入的参数node 就是init_heap 函数的init_region
     * 变量，即整个堆的起始位置
     */
    if (bin->head == NULL) {
        bin->head = node;
        return;
    }
    
    // we need to save next and prev while we iterate
    node_t *current = bin->head;
    node_t *previous = NULL;
    // iterate until we get the the end of the list or we find a 
    // node whose size is
    while (current != NULL && current->size <= node->size) {
        previous = current;
        current = current->next;
    }
    /* 如果一个块应该被插入到空闲链表的结尾 */
    if (current == NULL) { // we reached the end of the list
        /* 个人感觉此处可以加上一个node->next = previous->next */
        previous->next = node;
        node->prev = previous;
    }
    else {
        /* 块应该被插入到中间位置 */
        if (previous != NULL) { // middle of list, connect all links!
            node->next = current;
            previous->next = node;

            node->prev = previous;
            current->prev = node;
        }
        /* 块应该被插入到开头 */
        else { // head is the only element
            node->next = bin->head;
            /* 此处感觉可以加上一个node->prev = bin->head->prev */
            bin->head->prev = node;
            bin->head = node;
        }
    }
}

void remove_node(bin_t * bin, node_t *node) {
    /* 如果为空，那么说明没有可以移除的块 */
    if (bin->head == NULL) return; 
    /* 如果块在开头 */
    if (bin->head == node) { 
        bin->head = bin->head->next;
        return;
    }
    /* 处理块不在开头的情况 */
    node_t *temp = bin->head->next;
    while (temp != NULL) {
        /* 如果找到了块 */
        if (temp == node) { // found the node
            /* 如果块是最后一个 */
            if (temp->next == NULL) { // last item
                temp->prev->next = NULL;
            }
            /* 如果块在中间 */
            else { // middle item
                temp->prev->next = temp->next;
                temp->next->prev = temp->prev;
            }
            // we dont worry about deleting the head here because we already checked that
            return;
        }
        temp = temp->next;
    }
}

node_t *get_best_fit(bin_t *bin, size_t size) {
    if (bin->head == NULL) return NULL; // empty list!

    node_t *temp = bin->head;

    /* 找到了一个空闲链表 */
    while (temp != NULL) {
        /* 由于空闲链表中的块是按照大小从小到大排序，因此如果
         * 大小不合适就往后顺移到下一个块 
         */
        if (temp->size >= size) {
            return temp; // found a fit!
        }
        temp = temp->next;
    }
    return NULL; // no fit!
}

/* 获得最后一个块 */
node_t *get_last_node(bin_t *bin) {
    node_t *temp = bin->head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    return temp;
}

