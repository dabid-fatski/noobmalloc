#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#include "noobmalloc.h"

typedef struct block{
    // the blocks for each allocation
    size_t size;
    int free;
    struct block* next;
    struct block* prev;
} blocktype;

static blocktype* heap_head = NULL;

void* noob_malloc(size_t size){
    // makes sure it doesnt overflow
    if (size > SIZE_MAX - sizeof(blocktype)) {
        return NULL;
    }

    // if there's already one that exists, assign it
    blocktype* cur_heap = heap_head;
    blocktype* prev_heap = NULL;

    while (cur_heap != NULL){
        if (cur_heap->free == 1 && cur_heap->size >= size){
            // if we found open space, and we have more space than we need, split it
            // otherwise, just return cur_heap->free = 0;
            if (cur_heap->size >= size + sizeof(blocktype) + 16){
                // we want the address of the end of what we want set
                // so beginning (cur_heap + 1 block) + the size we want = the beginning of the split block
                blocktype* block = (blocktype*)((char*)(cur_heap + 1) + size);
                block->prev = cur_heap;
                block->next = cur_heap->next;
                block->size = cur_heap->size - size - sizeof(blocktype);
                block->free = 1;
                // check in case we are at last node
                if (cur_heap->next != NULL){
                    cur_heap->next->prev = block;
                }
                cur_heap->next = block;
                cur_heap->size = size;
                cur_heap->free = 0;
                return (void*)(cur_heap + 1);
            } else {
                cur_heap->free = 0;
                return (void*)(cur_heap + 1);
            }
        }
        cur_heap = cur_heap->next;
    }

    // otherwise we need to make a new block
    // if we don't have any heap allocated, create the first linked list node
    // else iterate to the end, and connect the end to the new one

    // get the current end of the heap address
    void* start = sbrk(0);
    
    // allocate size for the block
    size_t block_size = sizeof(blocktype);

    // get the total size we need
    size_t total_size = block_size + size;

    // set end of the heap
    // using char cuz char is 1 byte
    void* cur = (char*)start + total_size;

    // check if it allocated properly
    if (brk(cur) == -1){
        return NULL;
    }

    // make the beginning of the allocated memory the metadata and set stuff
    blocktype* block = (blocktype*)start;
    block->size = size;
    block->free = 0; // 0 = used, 1 = free
    block->next = NULL;

    if (heap_head == NULL){
        heap_head = block;
        block->prev = NULL;
    } else {
        // go to the end and add the new block
        blocktype* tail = heap_head;
        while (tail->next != NULL){
            tail = tail->next;
        } 
        tail->next = block;
        block->prev = tail;
    }

    // return the pointer to the beginning of the actual memory, not including the block
    return (void*)(block + 1);
}

void noob_free(void* pointer){
    // need to be able to free middle blocks
    // make a block(struct) that gets the size of an allocation and a check (0 or 1) if it's free or not
    
    // if there is no pointer, nothing to free
    if (pointer == NULL){
        return;
    }

    blocktype* cur_heap = (blocktype*)pointer - 1; 
    cur_heap->free = 1;
    
    // when a block is freed, check forward and backward if those addresses are adjacent and free
    // make this a while
    while (cur_heap->next != NULL && cur_heap->next->free == 1){
        blocktype* next_heap = cur_heap->next;

        // add size of next to current, set next to the block after next
        cur_heap->size += next_heap->size + sizeof(blocktype);
        cur_heap->next = next_heap->next;

        // if block after next isn't null, we can set the prev pointer
        if (cur_heap->next != NULL){
            cur_heap->next->prev = cur_heap;
        }
    }

    while (cur_heap->prev != NULL && cur_heap->prev->free == 1){
        // make prev to make it easier to read
        blocktype* prev_heap = cur_heap->prev;

        prev_heap->size += cur_heap->size + sizeof(blocktype);
        prev_heap->next = cur_heap->next;
        
        if (prev_heap->next != NULL){
            cur_heap->next->prev = prev_heap;    
        }
        cur_heap = prev_heap;
    }
}

void* noob_realloc(void* ptr, size_t size){
    // probably goes something along the lines of getting the size of what you passed so you get the start address
    // copy stuff into new memory
    // free the old memory
    
    // if no pointer, nothing to reallocate
    // was thinking could make this a malloc, but i feel like thats kinda unintuitive
    if (ptr == NULL){
        return NULL;
    }
    // if size is 0 then it's just getting rid of the block
    if (size == 0){
        noob_free(ptr);
        return NULL;
    }

    // get the current heap that is being reallocated
    blocktype* cur_heap = (blocktype*)ptr - 1;

    // if its the same size, just return ptr
    if (cur_heap->size == size){
        return ptr;
    }
    // if new size is less than cur size, return the same ptr
    if (cur_heap->size > size){
        return ptr;
    }

    //  otherwise create a new block, and copy everything over
    void* new_heap = noob_malloc(size);
    // check if it allocated properly
    if (new_heap == NULL){
        return NULL;
    }

    // get the min size of the two, because thats the most you can copy over
    size_t min_size = cur_heap->size < size ? cur_heap->size : size;
    // change ptr and new_heap pointers to char to copy bits 1 by 1
    char* cur = (char*)ptr;
    char* destination = (char*)new_heap;

    for (size_t i = 0; i < min_size; i++){
        destination[i] = cur[i];
    }

    // free the old memory
    noob_free(ptr);

    return new_heap;
}