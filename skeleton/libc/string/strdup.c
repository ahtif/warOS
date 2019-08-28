#include <string.h>

char *strdup(char *str) {
    return strndup(str, strlen(str));
}
