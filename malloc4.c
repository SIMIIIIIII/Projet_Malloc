#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define SIZE_HEAP 64000
uint8_t MY_HEAP[SIZE_HEAP];


//j'ai fait beaucoup de if dans des fonctions parce qu'il y avait beaucoup de faute de segmentation
//donc c'est possible que tu trouves de if inutiles

typedef struct {
    size_t size;     
    uint8_t is_free; // 1 si libre et 0 si occupé;
} Block;

Block* LIST_SMALL_FREE[8] = {NULL};
size_t SIZE_SMALL_FREE = 0;
Block* LIST_BIG_FREE[8] = {NULL};
size_t SIZE_BIG_FREE = 0;

/* return la premier le debut de la heap */
uint8_t* heap_start(){
    return MY_HEAP;
}

/* return la fin de la heap */
uint8_t* heap_end(){
    return MY_HEAP + SIZE_HEAP;
}

/* return la fin d'un bloc */
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

/* regarde si le block se trouve dans l'interval de la heap */
int in_heap(Block* block) {
    return ((uint8_t*)block >= heap_start()) && ((uint8_t*)block + sizeof(Block) <= heap_end());
}

Block* get_prev_block(Block* block) {
    if (!in_heap(block)) return NULL;

    // Si c'est le debut de la heap alors il n'y a pas des blocks précédents
    if ((uint8_t*)block == heap_start()) return NULL;

    // la fin du block précédent
    Block* end_Prev = (Block*)((uint8_t*)block - sizeof(Block));
    if ((uint8_t*)end_Prev < heap_start() || (uint8_t*)end_Prev + sizeof(Block) > heap_end()) {
        return NULL;
    }

    size_t prev_Size = end_Prev->size;
    if (prev_Size == 0 || prev_Size > SIZE_HEAP) return NULL;

    //on check l'adress du debut du block précédent.
    uint8_t* prev_Addr = (uint8_t*)end_Prev - sizeof(Block) - prev_Size;
    if (prev_Addr < heap_start()) return NULL;

    Block* prev = (Block*)prev_Addr;
    if (!in_heap(prev)) return NULL;
    
    //un ckeck de sécurité pour etre sur qu'on a vraiment le debut du block
    Block* check_End = end_of_block(prev);
    if (check_End != end_Prev) return NULL;

    return prev;
}

/* return le block suivant */
Block* get_next_block(Block* block) {
    if (!in_heap(block)) return NULL;

    Block* end = end_of_block(block);
    if ((uint8_t*)end + sizeof(Block) >= heap_end()) return NULL;

    Block* next = (Block*)((uint8_t*)end + sizeof(Block));
    if (!in_heap(next)) return NULL;
    
    //on verifie si la fin du next block se trouve dans le range de la heap
    Block* nextEnd = end_of_block(next);
    if ((uint8_t*)nextEnd + sizeof(Block) > heap_end()) return NULL;

    return next;
}

void remove_free_block(Block** list, size_t* size_list, size_t idx) {
    for (size_t j = idx; j < *size_list - 1; j++) {
        list[j] = list[j + 1];
    }
    (*size_list)--;
}

void add_free_block(Block** list, size_t* size_list, Block* block) {
    if (*size_list < 8) {
        list[*size_list] = block;
        (*size_list)++;
    }
}

void* allocate(size_t size, Block* block) {
    if (!in_heap(block)) return NULL;

    size_t old_size = block->size;
    block->is_free = 0;

    // on split si reste suffisant
    if (old_size >= size + 2 * sizeof(Block) + 1) {
        block->size = size;
        set_end_block(block);

        Block* next = (Block*)((uint8_t*)block + sizeof(Block) + size + sizeof(Block));
        next->is_free = 1;
        next->size = old_size - size - 2 * sizeof(Block);
        set_end_block(next);

        // on ajoute le bloc restant dans la bonne liste
        if (next->size > 1000) add_free_block(LIST_BIG_FREE, &SIZE_BIG_FREE, next);
        else add_free_block(LIST_SMALL_FREE, &SIZE_SMALL_FREE, next);
    } else {
        block->size = old_size;
        set_end_block(block);
    }
    return (void*)(block + 1);
}

void init(){
    Block* block = (Block*)MY_HEAP;
    block->size = SIZE_HEAP - (2 * sizeof(Block));
    block->is_free = 1;
    set_end_block(block);
    LIST_BIG_FREE[0] = block;
    SIZE_BIG_FREE++;
}

void my_free(void *pointer) {
    if (!pointer) return;

    Block* block = ((Block*)pointer) - 1;
    if (!in_heap(block)) return;

    block->is_free = 1;
    set_end_block(block);

    // fusion avec bloc précédent
    Block* prev = get_prev_block(block);
    if (prev && prev->is_free && get_next_block(prev) == block) {
        prev->size += block->size + 2 * sizeof(Block);
        prev->is_free = 1;
        set_end_block(prev);
        block = prev;
    }

    // fusion avec bloc suivant
    Block* next = get_next_block(block);
    if (next && next->is_free && get_prev_block(next) == block) {
        block->size += next->size + 2 * sizeof(Block);
        block->is_free = 1;
        set_end_block(block);
    }

    // réinsérer dans les listes rapides
    if (block->size > 1000) add_free_block(LIST_BIG_FREE, &SIZE_BIG_FREE, block);
    else add_free_block(LIST_SMALL_FREE, &SIZE_SMALL_FREE, block);
}


void *my_malloc(size_t size) {
    if (size == 0 || size > SIZE_HEAP) return NULL;

    // 1. Chercher dans les listes rapides
    if (size > 1000) {
        for (size_t i = 0; i < SIZE_BIG_FREE; i++) {
            if (LIST_BIG_FREE[i] && LIST_BIG_FREE[i]->is_free && LIST_BIG_FREE[i]->size >= size) {
                Block* block = LIST_BIG_FREE[i];
                remove_free_block(LIST_BIG_FREE, &SIZE_BIG_FREE, i);
                return allocate(size, block);
            }
        }
    } else {
        for (size_t i = 0; i < SIZE_SMALL_FREE; i++) {
            if (LIST_SMALL_FREE[i] && LIST_SMALL_FREE[i]->is_free && LIST_SMALL_FREE[i]->size >= size) {
                Block* block = LIST_SMALL_FREE[i];
                remove_free_block(LIST_SMALL_FREE, &SIZE_SMALL_FREE, i);
                return allocate(size, block);
            }
        }
    }

    // 2. Sinon, scan autour du bloc courant
    Block* right = (Block*)MY_HEAP;

    while (right != NULL) {
        if (right && right->is_free && right->size >= size) return allocate(size, right);
        right = get_next_block(right);
    }
    return NULL;
}

void etat_memoire(){
    uint16_t blocs_libres = 0; // nombre total de blocs libres rencontrés
    uint16_t blocs_occupes = 0; // nombre total de blocs occupés rencontrés
    uint16_t max_libre = 0; // taille du plus grand bloc libre (hors métadonnées)
    uint16_t total_libre = 0; // quantité totale de bytes libres (hors métadonnées)
    double indice_fragmentation;

    Block* block = (Block*)MY_HEAP;
    while ((uint8_t*)block < heap_end())
    {
        if (block->is_free){
            blocs_libres++;
            total_libre += block->size;
            if (max_libre < block->size) max_libre = block->size;
        }
        else blocs_occupes++;

        block = (Block*)((uint8_t*)block + block->size + sizeof(block)*2);
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