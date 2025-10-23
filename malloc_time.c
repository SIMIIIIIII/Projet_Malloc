#include <stdio.h>
#include <stdint.h>

#define SIZE_HEAP 64000
uint8_t MY_HEAP[SIZE_HEAP];

typedef struct BeginBlock{
    size_t size;
    uint8_t is_free; // 1 si libre et 0 si occupé;
    struct BeginBlock* next;
} BeginBlock;

typedef struct{
    size_t size;
    uint8_t is_free;
} EndBlock;


/* return le debut de la heap */
uint8_t* heap_start(){
    return MY_HEAP + sizeof(BeginBlock*);
}

/* return la fin de la heap*/
uint8_t* heap_end(){
    return MY_HEAP + SIZE_HEAP;
}

/* return la fin d'un bloc (adresse du EndBlock) */
EndBlock* end_of_block(BeginBlock* b) {
    return (EndBlock*)((uint8_t*)b + sizeof(BeginBlock) + b->size);
}

/* met les méta données de la fin du bloc */
void set_end_block(BeginBlock* block) {
    EndBlock* end_block = end_of_block(block);
    if ((uint8_t*)end_block >= heap_start() && (uint8_t*)end_block + sizeof(EndBlock) <= heap_end()) {
        end_block->size = block->size;
        end_block->is_free = block->is_free;
    }
}

/* regarde si le block se trouve dans le range de la heap */
int in_heap(BeginBlock* block) {
    if (block == NULL) return 0;
    uint8_t* bstart = (uint8_t*)block;
    if (bstart < heap_start()) return 0;

    // vérifier qu'au moins le header tient
    if (bstart + sizeof(BeginBlock) > heap_end()) return 0;
    return 1;
}

/* retourne le block précédent libre ou occupé */
BeginBlock* get_prev_block(BeginBlock* block) {
    if (!in_heap(block)) return NULL;
    if ((uint8_t*)block == heap_start()) return NULL;
    
    //verifier la fin de la liste précedente
    EndBlock* end_Prev = (EndBlock*)((uint8_t*)block - sizeof(EndBlock));
    if ((uint8_t*)end_Prev < heap_start() || (uint8_t*)end_Prev + sizeof(EndBlock) > heap_end()) {
        return NULL;
    }

    //verifier la taille de la liste précedente
    size_t prev_Size = end_Prev->size;
    if (prev_Size == 0 || prev_Size > SIZE_HEAP) return NULL;

    //verifier l'adress de la liste précédente si elle se trouve dans le range
    uint8_t* prev_Addr = (uint8_t*)end_Prev - sizeof(BeginBlock) - prev_Size;
    if (prev_Addr < heap_start()) return NULL;

    BeginBlock* prev = (BeginBlock*)prev_Addr;
    if (!in_heap(prev)) return NULL;

    EndBlock* check_End = end_of_block(prev);
    if (check_End != end_Prev) return NULL;

    return prev;
}

/* return le block suivant peut être NULL si dépassement */
BeginBlock* get_next_block(BeginBlock* block) {
    if (!in_heap(block)) return NULL;

    EndBlock* end = end_of_block(block);

    // le BeginBlock suivant commence après la EndBlock
    uint8_t* next_begin = (uint8_t*)end + sizeof(EndBlock);
    if (next_begin + sizeof(BeginBlock) > heap_end()) return NULL;

    BeginBlock* next = (BeginBlock*)next_begin;
    if (!in_heap(next)) return NULL;

    // verifier que le end du next est toujours dans la heap
    EndBlock* nextEnd = end_of_block(next);
    if ((uint8_t*)nextEnd + sizeof(EndBlock) > heap_end()) return NULL;

    return next;
}

/* trouve le précédent libre retourne NULL si aucun */
BeginBlock* get_prev_free(BeginBlock* block){
    if (!in_heap(block)) return NULL;

    // si block est au début, il n'y a pas de prev
    if ((uint8_t*)block == heap_start()) return NULL;

    BeginBlock* prev = block;

    while (1) {
        EndBlock* end_prev = (EndBlock*)((uint8_t*)prev - sizeof(EndBlock));
        if ((uint8_t*)end_prev < heap_start() || (uint8_t*)end_prev + sizeof(EndBlock) > heap_end()) {
            return NULL;
        }
        // adresse du début du block précédent
        uint8_t* prev_addr = (uint8_t*)end_prev - sizeof(BeginBlock) - end_prev->size;
        if (prev_addr < heap_start()) return NULL;
        prev = (BeginBlock*)prev_addr;
        if (!in_heap(prev)) return NULL;
        EndBlock* chk = end_of_block(prev);
        if (chk != end_prev) return NULL;
        if (prev->is_free) return prev;
        
        if ((uint8_t*)prev == heap_start()) return NULL;
    }
}

/* trouve le suivant libre retourne NULL si aucun */
BeginBlock* get_next_free(BeginBlock* block){
    if (!in_heap(block)) return NULL;

    BeginBlock* next = block;
    while (1) {
        // calculer adresse du suivant logique
        uint8_t* next_addr = (uint8_t*)next + sizeof(BeginBlock) + next->size + sizeof(EndBlock);
        if (next_addr + sizeof(BeginBlock) > heap_end()) return NULL;
        next = (BeginBlock*)next_addr;
        if (!in_heap(next)) return NULL;
        
        EndBlock* nextEnd = end_of_block(next);
        if ((uint8_t*)nextEnd + sizeof(EndBlock) > heap_end()) return NULL;
        if (next->is_free) return next;
    }
}

// Pour lire l'adresse stockée :
BeginBlock* get_first_free() {
    return *((BeginBlock**)MY_HEAP);
}

void init(){
    // Réserver de l'espace au début pour stocker l'adresse du premier block libre
    BeginBlock** first_free_ptr = (BeginBlock**)MY_HEAP;
    
    // Le premier bloc commence après cet espace réservé
    BeginBlock* block = (BeginBlock*)heap_start();
    block->size = SIZE_HEAP - sizeof(BeginBlock) - sizeof(EndBlock) - sizeof(BeginBlock*);
    block->is_free = 1;
    block->next = NULL;
    set_end_block(block);
    
    // Stocker l'adresse du premier bloc libre dans l'espace reservé
    *first_free_ptr = block;
}

void my_free(void *pointer) {
    if (!pointer) return;
    
    BeginBlock* block = ((BeginBlock*)pointer) - 1;
    if (!in_heap(block)) return;
    
    block->is_free = 1;
    block->next = NULL;
    set_end_block(block);
    
    // Fusion avec le bloc précédent si libre
    BeginBlock* prev = get_prev_block(block);
    if (prev && prev->is_free) {
        prev->size = prev->size + block->size + sizeof(BeginBlock) + sizeof(EndBlock);
        prev->is_free = 1;
        set_end_block(prev);
        block = prev;
    }
    
    // Fusion avec le bloc suivant si libre
    BeginBlock* next = get_next_block(block);
    if (next && next->is_free) {
        block->size = block->size + next->size + sizeof(BeginBlock) + sizeof(EndBlock);
        block->is_free = 1;
        set_end_block(block);
    }
    
    block->next = get_next_free(block);
    prev = get_prev_free(block);
    if (prev != NULL) prev->next = block;
    
    // Reconstruire toute la liste chaînée des blocs libres
    BeginBlock** first_free_ptr = (BeginBlock**)MY_HEAP;
    BeginBlock* first_free = get_first_free();
    if (first_free == NULL || (uint8_t*)block < (uint8_t*)first_free) *first_free_ptr = block;
}

void *my_malloc(size_t size) {
    if (size == 0) return NULL;
    if (size > SIZE_HEAP - sizeof(BeginBlock) - sizeof(EndBlock)) return NULL;

    BeginBlock* first_free = get_first_free();
    if (first_free == NULL) return NULL;

    BeginBlock** first_free_ptr = (BeginBlock**)MY_HEAP;

    if (!first_free->is_free) {
        *first_free_ptr = get_next_free((BeginBlock*)heap_start());
        first_free = get_first_free();
    }
    if (first_free == NULL) return NULL;

    BeginBlock* block = first_free;
    BeginBlock* prev_free = NULL;
    BeginBlock* prev_fit = NULL;

    // Recherche du meilleur bloc
    while (block != NULL) {
        if (!in_heap(block)) break;

        if (block->is_free && block->size >= size) {
            size_t old_size = block->size;

            // Vérifier si on peut diviser le bloc
            if (old_size >= size + sizeof(BeginBlock) + sizeof(EndBlock) + 1) {
                block->size = size;
                block->is_free = 0;
                set_end_block(block);
                
                // Créer un nouveau bloc libre après best_fit
                BeginBlock* next = (BeginBlock*)((uint8_t*)block + sizeof(BeginBlock) + size + sizeof(EndBlock));
                
                if ((uint8_t*)next + sizeof(BeginBlock) + sizeof(EndBlock) <= heap_end()) {
                    next->is_free = 1;
                    next->size = old_size - size - sizeof(BeginBlock) - sizeof(EndBlock);
                    next->next = block->next;
                    set_end_block(next);
                    
                    if (prev_fit != NULL) prev_fit->next = next;
                    else *first_free_ptr = next;
                }
                else{
                    if (prev_free != NULL) prev_free->next = block->next;
                    else *first_free_ptr = block->next;
                }
                block->next = NULL;
                return (void*)(block + 1);
            }
            // Allocation exacte, pas de division possible
            block->is_free = 0;
            set_end_block(block);
            if (prev_free != NULL) prev_free->next = block->next;
            else *first_free_ptr = block->next;
            block->next = NULL;
            return (void*)(block + 1);
        }
        prev_free = block;
        block = block->next;
    }
    return NULL;
}