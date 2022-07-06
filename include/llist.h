#ifndef LLIST_H
#define LLIST_H

#include "heap.h"
#include <stdint.h>

/* 将一个块添加回空闲链表 */
void add_node(bin_t *bin, node_t *node);

/* 将一个块从空闲链表中移除 */
void remove_node(bin_t *bin, node_t *node);

/* 找到大小最合适的块 */
node_t *get_best_fit(bin_t *list, size_t size);
/* 最后三个函数没用过 */
node_t *get_last_node(bin_t *list);

node_t *next(node_t *current);
node_t *prev(node_t *current);

#endif
