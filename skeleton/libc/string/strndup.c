#include <string.h>
#include <kernel/kheap.h>

char* strndup(char* str, size_t n) {
    size_t len = n + 1;
    char* dup = kmalloc(len * sizeof(char));
    if(dup) {
        memcpy(dup, str, n);
        dup[n] = '\0';
    }
    return dup;
}
