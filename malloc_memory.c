#include <stdio.h>
#include <stdint.h>

#define SIZE_HEAP 64000
uint8_t MY_HEAP[SIZE_HEAP];

typedef struct{
    size_t size;
    uint8_t is_free;
} Block;


/* return le debut de la heap */
uint8_t* heap_start(){
    return MY_HEAP;
}

/* return la fin de la heap*/
uint8_t* heap_end(){
    return MY_HEAP + SIZE_HEAP;
}

/* return la fin d'un bloc (adresse du EndBlock) */
Block* end_of_block(Block* b) {
    return (Block*)((uint8_t*)b + sizeof(Block) + b->size);
}

/* met les méta données de la fin du bloc */
void set_end_block(Block* block) {
    Block* end_block = end_of_block(block);
    if ((uint8_t*)end_block >= heap_start() && (uint8_t*)end_block + sizeof(Block) <= heap_end()) {
        end_block->size = block->size;
        end_block->is_free = block->is_free;
    }
}

/* regarde si le block se trouve dans le range de la heap */
int in_heap(Block* block) {
    if (block == NULL) return 0;
    uint8_t* bstart = (uint8_t*)block;
    if (bstart < heap_start()) return 0;

    // vérifier qu'au moins le header tient
    if (bstart + sizeof(Block) > heap_end()) return 0;
    return 1;
}

/* retourne le block précédent libre ou occupé */
Block* get_prev_block(Block* block) {
    if (!in_heap(block)) return NULL;
    if ((uint8_t*)block == heap_start()) return NULL;
    
    //verifier la fin de la liste précedente
    Block* end_Prev = (Block*)((uint8_t*)block - sizeof(Block));
    if ((uint8_t*)end_Prev < heap_start() || (uint8_t*)end_Prev + sizeof(Block) > heap_end()) {
        return NULL;
    }

    //verifier la taille de la liste précedente
    size_t prev_Size = end_Prev->size;
    if (prev_Size == 0 || prev_Size > SIZE_HEAP) return NULL;

    //verifier l'adress de la liste précédente si elle se trouve dans le range
    uint8_t* prev_Addr = (uint8_t*)end_Prev - sizeof(Block) - prev_Size;
    if (prev_Addr < heap_start()) return NULL;

    Block* prev = (Block*)prev_Addr;
    if (!in_heap(prev)) return NULL;

    Block* check_End = end_of_block(prev);
    if (check_End != end_Prev) return NULL;

    return prev;
}

/* return le block suivant peut être NULL si dépassement */
Block* get_next_block(Block* block) {
    if (!in_heap(block)) return NULL;

    Block* end = end_of_block(block);

    // le BeginBlock suivant commence après la EndBlock
    uint8_t* next_begin = (uint8_t*)end + sizeof(Block);
    if (next_begin + sizeof(Block) > heap_end()) return NULL;

    Block* next = (Block*)next_begin;
    if (!in_heap(next)) return NULL;

    // verifier que le end du next est toujours dans la heap
    Block* nextEnd = end_of_block(next);
    if ((uint8_t*)nextEnd + sizeof(Block) > heap_end()) return NULL;

    return next;
}

void init(){
    // Le premier bloc commence après cet espace réservé
    Block* block = (Block*)heap_start();
    block->size = SIZE_HEAP - (sizeof(Block) * 2);
    block->is_free = 1;
    set_end_block(block);
}

void my_free(void *pointer) {
    if (!pointer) return;
    
    Block* block = ((Block*)pointer) - 1;
    if (!in_heap(block)) return;
    
    block->is_free = 1;
    set_end_block(block);
    
    // Fusion avec le bloc précédent si libre
    Block* prev = get_prev_block(block);
    if (prev && prev->is_free) {
        prev->size = prev->size + block->size + (sizeof(Block)*2);
        prev->is_free = 1;
        set_end_block(prev);
        block = prev;
    }
    
    // Fusion avec le bloc suivant si libre
    Block* next = get_next_block(block);
    if (next && next->is_free) {
        block->size = block->size + next->size + (sizeof(Block)*2);
        block->is_free = 1;
        set_end_block(block);
    }
}

void *my_malloc(size_t size) {
    if (size == 0) return NULL;
    if (size > SIZE_HEAP - (sizeof(Block)*2)) return NULL;

    Block* block =(Block*) heap_start();
    Block* best_fit = NULL;

    // Recherche du meilleur bloc
    while (in_heap(block)) {
        if (!in_heap(block)) break;

        if (block->is_free && block->size >= size) {
            size_t old_size = block->size;

            // Vérifier si on peut diviser le bloc
            if (old_size >= size + (sizeof(Block)*2) + 1) {
                // Chercher le meilleur bloc
                if (best_fit == NULL || block->size < best_fit->size) best_fit = block;

            } else {
                // Allocation exacte, pas de division possible
                block->is_free = 0;
                set_end_block(block);
                return (void*)(block + 1);
            }
        }
        
        block = get_next_block(block);
    }

    // Si on a trouvé un best_fit, on le divise
    if (best_fit != NULL) {
        size_t old_size = best_fit->size;
        best_fit->size = size;
        best_fit->is_free = 0;
        set_end_block(best_fit);
        
        // Créer un nouveau bloc libre après best_fit
        Block* next = (Block*)((uint8_t*)best_fit + (sizeof(Block)*2) + size);
        
        if (in_heap(next)) {
            next->is_free = 1;
            next->size = old_size - size - (sizeof(Block)*2);
            set_end_block(next);
        }
        return (void*)(best_fit + 1);
    }
    return NULL;
}