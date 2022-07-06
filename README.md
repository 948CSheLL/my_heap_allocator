### heap_allocator - 一个简单的堆内存分配器

---

#### 项目特点

---

- 空闲链表(bin) 使用的是基于大小的双向链表；
- 可以合并释放的块(chunks) ；
- Q：空闲链表是根据大小排序的，采用快速最佳匹配；
- 实现了堆的扩张和收缩；
- 短小精悍，只有200 行。

#### 编译过程

---

通过下面的方式可以运行一个demo 。
```bash
$ cd /to/heap_allocator/path/
$ make gcc-test
```

#### 官方对于项目的解释

---

- 对于堆的初始化：

  为了初始化这个堆，必须提供一段内存，由`malloc`（通过堆分配堆）提供。而从操作系统设置的角度，则是需要将一些页面(pages) 映射并提供给堆（这是一种操作系统层面分配堆的情况）。请注意，`heap_t`结构中的空闲链表(bin) 也需要为它们分配内存。

  当调用函数 `init_heap` 时，必须提供空的heap 结构（带有分配的 bin 指针）的地址。然后 `init_heap` 函数将创建一个带有页眉（`node_t`结构）和页脚（`footer_t`结构）的块(chunk) 。为了确定这个块的大小，函数使用常量`HEAP_INIT_SIZE`，它将此添加到`start`参数中，以确定堆的结束位置。

  相关的代码如下：

  ```c
  // heap_allocator/main.c
  
  heap_t *heap = malloc(sizeof(heap_t));
  memset(heap, 0, sizeof(heap_t));
  
  void *region = malloc(HEAP_INIT_SIZE);
  memset(region, 0, HEAP_INIT_SIZE);
  
  for (i = 0; i < BIN_COUNT; i++) {
      heap->bins[i] = malloc(sizeof(bin_t));
      memset(heap->bins[i], 0, sizeof(bin_t));
  }
  init_heap(heap, (long) region);
  ```

- 元数据与设计：

  每个内存块在开头都有一个node 结构，在结尾有一个footer 结构。node 保存大小、块是否空闲以及双向链表中使用的两个指针（`next` 和 `prev`），如下所示：

  ```c
  // heap_allocator/include/heap.h
  
  typedef struct node_t {
      uint hole;				// 块是否空闲
      uint size;				// 块的大小
      struct node_t* next;
      struct node_t* prev;
  } node_t;
  ```

  页脚结构只是保存一个指向标头(header) 的指针（在释放相邻块时使用）。堆末尾的块称为“荒野(wilderness) ”块，它是最大的块，它的最小和最大大小在 `heap.h` 中定义，如下所示

  ```c
  // heap_allocator/include/heap.h
  
  #define MIN_WILDERNESS 0x2000
  #define MAX_WILDERNESS 0x1000000
  ```

  收缩和扩展堆是通过调整这个荒野块的大小来实现的。空闲的内存块存储在“bins”中，每个空闲链表(bin) 实际上只是一个具有相似大小的节点的双向链表。堆结构包含了堆空闲链表(bin) 数目的定义，如下所示

  ```c
  // heap_allocator/include/heap.h
  
  #define BIN_COUNT 9
  ```

  为了确定将块(chunk) 放置在哪个空闲链表(bin) 中，块的大小由函数`get_bin_index`映射到bin索引。这种一致的分配策略将确保可以以定义的方式访问和存储块。块在插入到空闲链表时被排序，因此块插入不是 O(1)，但这使得找到最适合的块变得更加容易。请注意，可以自定义分配空闲链表的函数，更复杂的分配函数可以帮助快速检索块。

- 分配：

  函数 `heap_alloc` 获取要分配的堆结构的地址和大小。该函数简单地使用 get_bin_index 来确定这个大小的块应该在哪里，当然可能没有那个大小的块。如果在相应的空闲链表(bin) 中没有找到合适的块，则将检查下一个空闲链表(bin) ，这将一直持续到找到一个块，或者到达最后一个空闲链表(bin) ，在这种情况下，将从荒野块(wilderness chunk) 中取出一段内存。如果找到的块足够大，那么它将被拆分。为了确定是否应该拆分一个块，如果将找到的块的大小减去需要的大小所得的值大于或等于`MIN_ALLOC_SZ`那么这意味着我们应该拆分这个块并将剩下的部分放在空闲链表中相应的位置。Q：一旦我们准备好返回我们找到的块，我们就获取该`next`字段的地址并返回它。这样做是因为在分配块时`next`和`prev`字段未使用，因此块的用户可以将数据写入这些字段而不会影响堆的内部工作。

- 释放：

  函数 `heap_free` 采用 `heap_alloc` 返回的指针。它减去正确的偏移量以获得节点结构的地址。不是简单地将块放入正确的空闲链表(bin) 中，而是检查提供的块周围的块。如果这些块中的任何一个是空闲的，那么我们可以合并这些块以创建更大的块。为了合并块，页脚用于获取前一个块的节点结构和下一个块的节点结构。例如，假设我们有一个名为`to_free` 的块，为了得到这个块之前的块，我们减去`sizeof(footer_t)`以获得前一个块的页脚。Q：页脚包含一个指向前一个块的头部的指针。要获取下一个块，我们只需获取`to_free`的页脚，然后添加`sizeof(footer_t)` 来获得下一个块。一旦完成所有这些并重新计算大小，该块将被放回一个 bin 中。

