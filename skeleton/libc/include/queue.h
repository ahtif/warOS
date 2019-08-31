#ifndef _QUEUE_H
#define _QUEUE_H 1

#include <sys/cdefs.h>
#include <stdbool.h>
#include <stddef.h>
#include <linkedlist.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef linkedlist_t queue_t;

queue_t* queue_create();
bool queue_enqueue(queue_t *queue, void *ptr);
void* queue_dequeue(queue_t *queue);
size_t queue_size(queue_t* queue);

#ifdef __cplusplus
}
#endif

#endif
