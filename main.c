#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "noobmalloc.h"

int main(void) {
    // Test 1: malloc
    void *p1 = noob_malloc(20);
    printf("Test 1 (malloc): %s\n", p1 != NULL ? "Worked" : "Failed");

    // Test 2: write and read memory
    if (p1 != NULL) {
        strcpy(p1, "hello");
        printf("Test 2 (read/write): %s\n",
               strcmp(p1, "hello") == 0 ? "Worked" : "Failed");
    } else {
        printf("Test 2 (read/write): Failed\n");
    }

    // Test 3: free
    noob_free(p1);
    printf("Test 3 (free): %s\n", "Worked");

    // Test 4: realloc
    char *p2 = noob_malloc(10);
    if (p2 != NULL) {
        strcpy(p2, "hello");
        char *p3 = noob_realloc(p2, 50);
        printf("Test 4 (realloc): %s\n",
               p3 != NULL && strcmp(p3, "hello") == 0
                   ? "Worked" : "Failed");
        noob_free(p3);
    } else {
        printf("Test 4 (realloc): Failed\n");
    }

    // Test 5: multiple allocations and frees
    void *a = noob_malloc(30);
    void *b = noob_malloc(60);
    void *c = noob_malloc(90);

    int passed = a != NULL && b != NULL && c != NULL;

    if (passed) {
        noob_free(b);
        noob_free(a);
        noob_free(c);
    }

    printf("Test 5 (multiple allocations): %s\n",
           passed ? "Worked" : "Failed");

    return 0;
}