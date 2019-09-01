#include <linkedlist.h>
#include <kernel/kheap.h>

node_t *make_node(void *ptr, node_t *prev, node_t *next);

node_t* get_node(linkedlist_t *list, uint32_t i);

linkedlist_t *list_create() {
    linkedlist_t* list = kmalloc(sizeof(linkedlist_t));
    if(list) {
        list->head = list->tail = NULL;
        list->size = 0;
    }
    return list;
}

bool list_add(linkedlist_t *list, void *ptr) {
    if(list->size == INT32_MAX)
      return false;
    node_t* node = make_node(ptr, list->tail, NULL);
    if(!node)
      return false;
    if(list->head && list->tail)
      list->tail->next = node;
    else
      list->head = node;
    list->tail = node;
    list->size++;
    return true;
}

void *list_get(linkedlist_t *list, uint32_t i) {
    if(list->size <= i) return NULL;
      return get_node(list, i)->data;
}

node_t* get_node(linkedlist_t *list, uint32_t i) {
    node_t* node = list->head;
    for (uint32_t j = 0; j < i; ++j)
      node = node->next;
    return node;
}

size_t list_size(linkedlist_t *list) {
    return list->size;
}

bool list_remove(linkedlist_t *list, uint32_t i) {
    if(list->size <= i)
      return false;
    node_t* node = get_node(list, i);
    if(node->next)
      node->next->prev = node->prev;
    if(node->prev)
      node->prev->next = node->next;
    if(i == 0)
      list->head = node->next;
    if(i == list->size - 1)
      list->tail = node->prev;
    list->size--;
    kfree(node);
    return true;
}

bool list_insert(linkedlist_t *list, void *ptr, uint32_t i) {
    if(list_size(list) >= UINT32_MAX || i > list_size(list))
        return false;
    node_t* node = make_node(ptr, NULL, NULL);
    if(!node) return false;
    if(i == 0) {
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    } else if (i == list_size(list))
      list_add(list, ptr);
    else {
        node_t* prev_node = get_node(list, i - 1), * next_node = prev_node->next;
        prev_node->next = node;
        node->prev = prev_node;
        next_node->prev = node;
        node->next = prev_node;
    }
    list->size++;
    return 0;
}

node_t *make_node(void *ptr, node_t *prev, node_t *next) {
    node_t* node = kmalloc(sizeof(node_t));
    if(node) {
        node->next = next;
        node->prev = prev;
        node->data = ptr;
    }
    return node;
}

bool sorted_list_add(linkedlist_t *list, void *ptr, comparator_t cmp) {
    uint32_t i = 0;
    while (i < list_size(list) && cmp(ptr, list_get(list, i)) > 0)
      i++;
    return list_insert(list, ptr, i);
}
