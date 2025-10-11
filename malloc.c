#include <stdio.h>
#include <stdint.h>
#include <string.h> 

#define SIZE_HEAP 64000
uint8_t MY_HEAP[SIZE_HEAP];

typedef struct {
    size_t size;
    uint8_t is_free;
    void* nextPart;
} Block;

Block* CURRENT;

void init(void){
    Block* block = (Block*)MY_HEAP;
    block->size = SIZE_HEAP - (sizeof(Block)*2);
    block->is_free = 1;
    block->nextPart = NULL;

    Block* endBlock = (Block*)(MY_HEAP + SIZE_HEAP - sizeof(Block));
    endBlock->size = SIZE_HEAP - (sizeof(Block)*2);
    endBlock->is_free = 1;
    CURRENT = block;
}

void endBlock(Block* block, size_t size, uint8_t is_free){
    Block* endBlock = (Block*)((uint8_t*)block + sizeof(Block) + size);
    endBlock->size = size;
    endBlock->is_free = is_free;
}

void allocate(size_t size, Block *block, Block* next_part){
    block->size = size;
    block->is_free = 0;
    block->nextPart = (void*) next_part;
    endBlock(block, size, 0);
    CURRENT = block;
}

void prepareNextBlock(Block* block, size_t totalSize, size_t size){
    Block *nextBlock = (Block*)((uint8_t*)block + (sizeof(Block)*2) + size);
    nextBlock->is_free = 1;
    nextBlock->size = totalSize - size - (sizeof(Block)*2);
    endBlock(nextBlock, nextBlock->size, 1);
    CURRENT = nextBlock;
}

void* fragmentAllocation(size_t size, Block* firstBlock, int requiredBlocks){
    Block* blocks[requiredBlocks];
    blocks[0] = firstBlock;
    int i = 1;
    
    while ((uint8_t*)firstBlock < MY_HEAP + SIZE_HEAP && i <requiredBlocks)
    {
        if (firstBlock->is_free == 1){
            blocks[i] = firstBlock;
            i++;
        }
        firstBlock = (Block*)((uint8_t*)firstBlock + (sizeof(Block)*2) + firstBlock->size);
    }

    for (size_t j = 0; j < requiredBlocks; j++)
    {
        if (j == requiredBlocks-1)
        {
            allocate(size, blocks[j], NULL);
            prepareNextBlock(blocks[j], blocks[j]->size, size);
        }
        else{
            allocate(blocks[j]->size, blocks[j], blocks[j+1]);
            size -= blocks[j]->size;
        }
    }
    return (void*)(blocks[0] + 1);
}

void* my_malloc(size_t size){
    Block* block = NULL;
    Block* toLeft = CURRENT;
    Block* toRight = CURRENT;
    int requiredBlocks = 0;
    Block* firstLeft = NULL;
    Block* firstRignt = NULL;
    size_t calculateSize = 0;

    while ((uint8_t*)toLeft >= MY_HEAP || (uint8_t*)toRight < MY_HEAP + SIZE_HEAP)
    {
        if (toLeft->is_free == 1 && toLeft->size >= size)
        {
            block = toLeft;
            break;
        }
        if (toRight->is_free == 1 && toRight->size >= size)
        {
            block = toRight;
            break;
        }
        if (toLeft->is_free == 1 && calculateSize < size){
             calculateSize += toLeft->size;
             requiredBlocks++;
             firstLeft = toLeft;
        }
        if (toRight->is_free == 1 && calculateSize)
        {
            if (firstRignt == NULL) firstRignt = toRight;
            calculateSize += toRight->size;
            requiredBlocks++;
        }
        Block* endPrev = toLeft - 1;
        toLeft = (Block*)((uint8_t*)toLeft - (sizeof(Block)*2) - endPrev->size);
        toRight = (Block*)((uint8_t*)toRight + (sizeof(Block)*2) + toRight->size);
    }

    if (block == NULL){
        if (calculateSize < size) return NULL;
        if (firstLeft == NULL) return fragmentAllocation(size, firstRignt, requiredBlocks);
        return fragmentAllocation(size, firstLeft, requiredBlocks); 
    }

    size_t saveSize = block->size;
    allocate(size, block, NULL);

    if (size + (sizeof(Block)*2) <= saveSize) prepareNextBlock(block, saveSize, size);
    
    return (void*)(block + 1);
}

void my_free(void *ptr){
}

/*Juste pour tester si la taille de la heap du départ change*/
int leftMemory(){
    Block* block = (Block*)MY_HEAP;
    size_t x = 0;
    int freeMemory = 0;

    while (x < SIZE_HEAP) {
        if ((uint8_t*)block < MY_HEAP || (uint8_t*)block >= MY_HEAP + SIZE_HEAP) break;
        if (block->size > SIZE_HEAP) break;

        if (block->is_free == 1) freeMemory += (int)block->size;

        x += sizeof(Block) + block->size + sizeof(Block);
        if (x >= SIZE_HEAP) break;
        block = (Block*)(MY_HEAP + x);
    }
    return freeMemory;
}

int main(void) {
    init();

    int x = leftMemory();
    printf("memoire initial: %d\n", x);

    char *str1 = (char*) my_malloc(20);
    char *str2 = (char*) my_malloc(20);
    char *str3 = (char*) my_malloc(50);
    char *str4 = (char*) my_malloc(100);
    char *str5 = (char*) my_malloc(1000);

    x = leftMemory();
    printf("memoire apres allocs: %d\n", x);

    my_free(str4);
    x = leftMemory();
    printf("memoir apres free str4: %d\n", x);

    char *str6 = (char*) my_malloc(2000);
    x = leftMemory();
    printf("memoire apres alloc 2000: %d\n", x);

    return 0;
}