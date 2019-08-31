#include <queue.h>

queue_t *queue_create() {
    return list_create();
}

bool queue_enqueue(queue_t *queue, void *ptr) {
    return list_add(queue, ptr);
}

void *queue_dequeue(queue_t *queue) {
    if(list_size(queue) >= 1) {
        void *item = list_get(queue, 0);
        list_remove(queue, 0);
        return item;
    }
    return NULL;
}

size_t queue_size(queue_t *queue) {
    return list_size(queue);
}
