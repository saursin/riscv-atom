#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

extern int _start_heap[2];  // specified as array to prevent a bounds warning

#define HEAP_START &_start_heap
#ifndef HEAP_SIZE
    #define HEAP_SIZE  1024
#endif

struct heapblock{
    struct heapblock * next;
    unsigned size;
};
typedef struct heapblock HeapBlock;

// Free list 
static HeapBlock * _freelist = NULL;


void heap_init() {
    _freelist = (HeapBlock *) HEAP_START;
    _freelist->next = NULL;
    _freelist->size = HEAP_SIZE - sizeof(HeapBlock);
}


void * malloc(unsigned nbytes) {
    // First Fit
    if(nbytes == 0  )
        return NULL;

    HeapBlock * curr = _freelist;
    HeapBlock * prev = NULL;

    while(curr != NULL){
        if(curr->size >= nbytes){
            if(curr->size > sizeof(HeapBlock) + nbytes){
                // split
                HeapBlock *newblk = (HeapBlock *)(((uint8_t*)curr) + sizeof(HeapBlock) + nbytes);
                newblk->next = curr->next;
                newblk->size = curr->size - (sizeof(HeapBlock) + nbytes);
                curr->next = newblk;
                curr->size = nbytes;
            }

            // allocate current block (remove from list & return chunk pointer)
            if(prev == NULL) {
                // first block
                _freelist = curr->next;
            } 
            else {
                prev->next = curr->next;
            }
            
            // printf("Allocating [0x%08x: 0x%08x, 0x%d]\n", (uint32_t)curr, (uint32_t)curr->next, curr->size);
            return (void *)(((uint8_t*)curr) + sizeof(HeapBlock));
        }
        prev = curr;
        curr = curr->next;
    }

    // No suitable block found
    return NULL;
}


void free(void * chunk){
    if(chunk == NULL)
        return;
    
    HeapBlock * blk = (HeapBlock *)(((uint8_t*)chunk) - sizeof(HeapBlock));
    // printf("Freeing    [0x%08x: 0x%08x, 0x%d]\n", (uint32_t)blk, (uint32_t)blk->next, blk->size);

    HeapBlock * curr = _freelist;
    HeapBlock * prev = NULL;

    // march till we reach location of blk
    while(curr != NULL && curr < blk){
        prev = curr;
        curr = curr->next;
    }

    // Insert block
    if (prev == NULL) {
        _freelist = blk;
    } else {
        prev->next = blk;
    }
    blk->next = curr;

    // // merge with previous
    // // only if prev and current are adjascent (which means both are free)
    // if(prev != NULL && ((uint8_t*)prev + sizeof(HeapBlock) + prev->size) == (uint8_t*)blk) {
    //     prev->size += sizeof(HeapBlock) + blk->size;
    //     prev->next = blk->next;
    // }

    // // merge with next
    // // only if current and next are adjascent (which means both are free)
    // if(curr != NULL && ((uint8_t*)blk + sizeof(HeapBlock) + blk->size) == (uint8_t*)curr) {
    //     blk->size += sizeof(HeapBlock) + curr->size;
    //     blk->next = curr->next;
    // }
}


void heap_defrag(){
    HeapBlock * curr = _freelist;
    HeapBlock * prev = NULL;

    while(curr != NULL){
        // merge if prev blk & curr blk are contigous (both free)
        if(prev != NULL && (uint8_t*)prev+sizeof(HeapBlock)+prev->size == (uint8_t*)curr){
            prev->size += sizeof(HeapBlock) + curr->size;
            prev->next = curr->next;
            curr = prev->next;
            continue;
        }
        prev = curr;
        curr = curr->next;
    }
}


void heap_status(){
    unsigned total_sz = 0;
    unsigned blk_cnt = 0;
    unsigned largest_blk_sz = 0;
    HeapBlock * currblk = _freelist;
    while(currblk != NULL) {
        printf("Block [addr=0x%08x: next=0x%08x, size=%d]\n", (uint32_t)currblk, (uint32_t)currblk->next, currblk->size);
        total_sz += currblk->size;
        if(currblk->size > largest_blk_sz){
            largest_blk_sz = currblk->size;
        }
        currblk = currblk->next;
        blk_cnt ++;
    }
    printf("Chunks: %d\n", blk_cnt);
    printf("Avail:  %d/%d bytes\n", total_sz, HEAP_SIZE);
    
    // see: https://cpp4arduino.com/2018/11/06/what-is-heap-fragmentation.html
    printf("Frag:   %.2f %%\n", (1.0 - ((float)largest_blk_sz/(float)HEAP_SIZE)) * 100);
}
