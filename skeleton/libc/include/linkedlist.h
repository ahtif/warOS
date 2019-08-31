#ifndef _LINKEDLIST_H
#define _LINKEDLIST_H 1

#include <sys/cdefs.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct node {
    void *data;
    struct node *next, *prev;
} node_t;

typedef struct linkedlist {
    node_t *head, *tail;
    size_t size;
} linkedlist_t;

linkedlist_t* list_create();
bool list_add(linkedlist_t* list, void *ptr);
bool list_insert(linkedlist_t *list, void *ptr, uint32_t i);
void* list_get(linkedlist_t* list, uint32_t i);
size_t list_size(linkedlist_t* list);
bool list_remove(linkedlist_t *list, uint32_t i);


#ifdef __cplusplus
}
#endif

#endif
