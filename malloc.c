#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

BeginBlock* FIST_FREE;

/* return le debut de la heap */
uint8_t* heap_start(){
    return MY_HEAP;
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

void init(){
    BeginBlock* block = (BeginBlock*)MY_HEAP;
    block->size = SIZE_HEAP - sizeof(BeginBlock) - sizeof(EndBlock);
    block->is_free = 1;
    block->next = NULL;
    set_end_block(block);
    FIST_FREE = block;
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
    
    // Reconstruire toute la liste chaînée des blocs libres
    FIST_FREE = NULL;
    BeginBlock* last_free = NULL;
    BeginBlock* current = (BeginBlock*)MY_HEAP;
    
    while ((uint8_t*)current + sizeof(BeginBlock) <= heap_end()) {
        if (!in_heap(current)) break;
        
        if (current->is_free) {
            current->next = NULL;
            
            if (FIST_FREE == NULL) {
                FIST_FREE = current;
            } else if (last_free != NULL) {
                last_free->next = current;
            }
            last_free = current;
        }
        
        // Passer au bloc suivant
        uint8_t* next_addr = (uint8_t*)current + sizeof(BeginBlock) + current->size + sizeof(EndBlock);
        if (next_addr >= heap_end()) break;
        current = (BeginBlock*)next_addr;
    }
}

void *my_malloc(size_t size) {
    if (size == 0) return NULL;
    if (size > SIZE_HEAP - sizeof(BeginBlock) - sizeof(EndBlock)) return NULL;
    if (FIST_FREE == NULL) return NULL;

    if (!FIST_FREE->is_free) FIST_FREE = get_next_free((BeginBlock*)MY_HEAP);
    if (FIST_FREE == NULL) return NULL;

    BeginBlock* block = FIST_FREE;
    BeginBlock* prev_free = NULL;
    BeginBlock* best_fit = NULL;
    BeginBlock* prev_fit = NULL;

    // Recherche du meilleur bloc
    while (block != NULL) {
        if (!in_heap(block)) break;

        if (block->is_free && block->size >= size) {
            size_t old_size = block->size;

            // Vérifier si on peut diviser le bloc
            if (old_size >= size + sizeof(BeginBlock) + sizeof(EndBlock) + 1) {
                // Chercher le meilleur bloc
                if (best_fit == NULL || block->size < best_fit->size) {
                    best_fit = block;
                    prev_fit = prev_free;
                }
            } else {
                // Allocation exacte, pas de division possible
                block->is_free = 0;
                set_end_block(block);

                if (prev_free != NULL) prev_free->next = block->next;
                else FIST_FREE = block->next;
                
                return (void*)(block + 1);
            }
        }
        prev_free = block;
        block = block->next;
    }

    // Si on a trouvé un best_fit, on le divise
    if (best_fit != NULL) {
        size_t old_size = best_fit->size;
        best_fit->size = size;
        best_fit->is_free = 0;
        set_end_block(best_fit);
        
        // Créer un nouveau bloc libre après best_fit
        BeginBlock* next = (BeginBlock*)((uint8_t*)best_fit + sizeof(BeginBlock) + size + sizeof(EndBlock));
        
        if ((uint8_t*)next + sizeof(BeginBlock) + sizeof(EndBlock) <= heap_end()) {
            next->is_free = 1;
            next->size = old_size - size - sizeof(BeginBlock) - sizeof(EndBlock);
            next->next = best_fit->next;
            set_end_block(next);
            
            if (prev_fit != NULL) prev_fit->next = next;
            else FIST_FREE = next;
        } else {
            if (prev_fit != NULL) prev_fit->next = best_fit->next;
            else FIST_FREE = best_fit->next;
        }
        
        return (void*)(best_fit + 1);
    }
    
    return NULL;
}

void etat_memoire(){
    uint16_t blocs_libres = 0; // nombre total de blocs libres rencontrés
    uint16_t blocs_occupes = 0; // nombre total de blocs occupés rencontrés
    uint16_t max_libre = 0; // taille du plus grand bloc libre (hors métadonnées)
    uint16_t total_libre = 0; // quantité totale de bytes libres (hors métadonnées)
    double indice_fragmentation;

    BeginBlock* block = (BeginBlock*)MY_HEAP;
    while ((uint8_t*)block < heap_end())
    {
        if (block->is_free){
            blocs_libres++;
            total_libre += block->size;
            if (max_libre < block->size) max_libre = block->size;
        }
        else blocs_occupes++;

        block = (BeginBlock*)((uint8_t*)block + block->size + sizeof(BeginBlock) + sizeof(EndBlock));
    }
    

    if (total_libre == 0) indice_fragmentation = 0.0;  
    else indice_fragmentation = 1.0 - ((double)max_libre / (double)total_libre);


    printf ("Etat de la mémoire : \n");
    printf("Nombre de blocs libres : %u\n",blocs_libres);
    printf("Nombre de blocs occupés : %u\n",blocs_occupes);
    printf("Taille du plus grand bloc libre : %u\n", max_libre);
    printf("Mémoire totale libre restante : %u\n",total_libre);
    printf("Indice de fragmentation de la mémoire : %.3f\n1 = très fragmentée, 0 = très compacte\n",indice_fragmentation) ;
}