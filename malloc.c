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

typedef struct {
    size_t size;
    uint8_t is_free;
    Block* begin_block;
} EndBlock;

void init(void){
    Block* block = (Block*)MY_HEAP;
    block->size = SIZE_HEAP - sizeof(Block) - sizeof(EndBlock);
    block->is_free = 1;
    block->nextPart = NULL;

    EndBlock* endBlock = (EndBlock*)(MY_HEAP + SIZE_HEAP - sizeof(EndBlock));
    endBlock->size = SIZE_HEAP - sizeof(Block) - sizeof(EndBlock);
    endBlock->is_free = 1;
    endBlock->begin_block = block;

}

void endBlock(Block* block, size_t size, uint8_t is_free){
    EndBlock* endBlock = (EndBlock*)((uint8_t*)block + sizeof(Block) + size);
    endBlock->size = size;
    endBlock->is_free = is_free;
    endBlock->begin_block = block;
}

void allocate(size_t size, Block *block, Block* next_part){
    block->size = size;
    block->is_free = 0;
    block->nextPart = (void*) next_part;
    endBlock(block, size, 0);
}

void prepareNextBloc(Block* block, size_t totalSize, size_t size){
    Block *nextBlock = (Block*)((uint8_t*)block + sizeof(Block) + size + sizeof(EndBlock));
    nextBlock->is_free = 1;
    nextBlock->size = totalSize - size - sizeof(Block) - sizeof(EndBlock);
    endBlock(nextBlock, nextBlock->size, 1);
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
        firstBlock = (Block*)((uint8_t*)firstBlock + sizeof(Block) + firstBlock->size, sizeof(EndBlock));
    }

    for (size_t j = 0; j < requiredBlocks; j++)
    {
        if (j == requiredBlocks-1)
        {
            allocate(size, blocks[j], NULL);
            prepareNextBloc(blocks[j], blocks[j]->size, size);
        }
        else{
            allocate(blocks[j]->size, blocks[j], blocks[j+1]);
            size -= blocks[j]->size;
        }
        
    }
    return (void*)(blocks[0] + 1);
    
}

void* my_malloc(size_t size){
    Block* block = (Block*)MY_HEAP;
    size_t x = 0;
    int requiredBlocks = 0;
    Block* firstFreeBlock = NULL;
    size_t calculateSize = 0;

    while (x < SIZE_HEAP && (block->is_free == 0 || block->size < size)) {
        if (block->size > SIZE_HEAP) return NULL;

        if (block->is_free == 1){
            if (calculateSize < size){
                calculateSize += block->size;
                requiredBlocks++;
            }
            if (firstFreeBlock == NULL) firstFreeBlock = block;
        }
        
        x += sizeof(Block) + block->size + sizeof(EndBlock);
        if (x >= SIZE_HEAP) break;
        block = (Block*)(MY_HEAP + x);
    }

    if (x >= SIZE_HEAP && calculateSize >= size) {
        return fragmentAllocation(size, firstFreeBlock, requiredBlocks);
    }

    size_t saveSize = block->size;
    allocate(size, block, NULL);

    if (size + sizeof(Block) + sizeof(EndBlock) <= saveSize)
    {
        prepareNextBloc(block, saveSize, size);
    }
    
    return (void*)(block + 1);
}

void combinePrev(Block* block, EndBlock* endBlock){
    EndBlock* endPrevBlock = ((EndBlock*)block) - 1;
    if ((uint8_t*)endPrevBlock < MY_HEAP || (uint8_t*)endPrevBlock >= MY_HEAP + SIZE_HEAP) return;
    if (endPrevBlock->is_free == 1)
    {
        Block* prevBlock = (Block*)((uint8_t*)endPrevBlock - endPrevBlock->size - sizeof(Block));
        if ((uint8_t*)prevBlock < MY_HEAP || (uint8_t*)prevBlock >= MY_HEAP + SIZE_HEAP) return;

        if (prevBlock->is_free == 0) return;
        endBlock->begin_block = prevBlock;
        prevBlock->size += sizeof(EndBlock) + sizeof(Block) + block->size;
        endBlock->size = prevBlock->size;
    }
}

void combineNext(Block* block, EndBlock* endBlock){
    Block* nextBlock = (Block*)(endBlock + 1);
    if ((uint8_t*)nextBlock < MY_HEAP || (uint8_t*)nextBlock >= MY_HEAP + SIZE_HEAP) return;

    if (nextBlock->is_free == 1)
    {
        block->size += sizeof(EndBlock) + sizeof(Block) + nextBlock->size;
        EndBlock* endNextBlock = (EndBlock*) ((uint8_t*)nextBlock + sizeof(Block) + nextBlock->size);

        if ((uint8_t*)endNextBlock < MY_HEAP || (uint8_t*)endNextBlock >= MY_HEAP + SIZE_HEAP) return;
        endNextBlock->size = block->size;
        endNextBlock->begin_block = block;
    }
}


void my_free(void *ptr){
    Block* block = ((Block*)ptr) - 1;
    if ((uint8_t*)block < MY_HEAP || (uint8_t*)block >= MY_HEAP + SIZE_HEAP) return;
    block->is_free = 1;

    EndBlock* endBlock = (EndBlock*)((uint8_t*)block + sizeof(block) + block->size);
    if ((uint8_t*)endBlock < MY_HEAP || (uint8_t*)endBlock >= MY_HEAP + SIZE_HEAP) return;
    endBlock->is_free = 1;

    if (block->nextPart != NULL) my_free(block->nextPart);
    
    block->nextPart = NULL;
    combinePrev(block, endBlock);
    combineNext(block, endBlock);
}

int leftMemory(){
    Block* block = (Block*)MY_HEAP;
    size_t x = 0;
    int freeMemory = 0;

    while (x < SIZE_HEAP) {
        if ((uint8_t*)block < MY_HEAP || (uint8_t*)block >= MY_HEAP + SIZE_HEAP) break;
        if (block->size > SIZE_HEAP) break;

        if (block->is_free == 1) freeMemory += (int)block->size;

        x += sizeof(Block) + block->size + sizeof(EndBlock);
        if (x >= SIZE_HEAP) break;
        block = (Block*)(MY_HEAP + x);
    }
    return freeMemory;
}


int main(void) {
    init();

    int x = leftMemory();
    printf("left initial: %d\n", x);

    char *str1 = (char*) my_malloc(20);
    char *str2 = (char*) my_malloc(20);
    char *str3 = (char*) my_malloc(50);
    char *str4 = (char*) my_malloc(100);
    char *str5 = (char*) my_malloc(1000);

    x = leftMemory();
    printf("left after allocs: %d\n", x);

    my_free(str4);
    x = leftMemory();
    printf("left after free str4: %d\n", x);

    char *str6 = (char*) my_malloc(2000);
    x = leftMemory();
    printf("left after alloc 2000: %d\n", x);

    return 0;
}