#ifndef noobmalloc
#define noobmalloc

#include <stddef.h>
#include <unistd.h>

void* noob_malloc(size_t size);
void noob_free(void* ptr);
void* noob_realloc(void* ptr, size_t size);

#endif