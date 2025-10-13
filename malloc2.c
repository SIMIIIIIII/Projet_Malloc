#include <stdint.h>
#include <stdio.h>
#include "malloc2.h"

///////////////////////////////////////
//   FONCTIONS CRUCIALES             //
//   à copier-coller dans Inginious  //
///////////////////////////////////////

void init(){
    //métadonnées d'initialisation : 
    ecrire_MD(2,SIZE_HEAP-4);
    ecrire_MD(SIZE_HEAP,SIZE_HEAP-4);
}

void my_free(void *pointer){
    // récupérer l'index du tableau
    uint8_t *addr = (uint8_t*) pointer;
    uint16_t index = addr - MY_HEAP;           

    // indiquer que le segment est désormais libre
    uint16_t taille = lire_MD(index)-1;         

    ecrire_MD(index,taille);
    ecrire_MD(index+taille+2,taille);

    // vérifier si le segment à droite est libre aussi et, si oui, fusionner les deux segments
    uint16_t index_d = index + taille + 4; 
    if (index_d <= SIZE_HEAP-2){
        uint16_t md_d = lire_MD(index_d); 

        if (md_d%2 == 0){
            uint16_t taille_totale_d = taille + md_d + 4; 
            ecrire_MD(index,taille_totale_d);
            ecrire_MD(index+taille_totale_d+2,taille_totale_d);
            taille = taille_totale_d; 
        }
    }

    // vérifier si le segment à gauche est libre aussi et, si oui, fusionner les deux segments
    uint16_t index_g = index - 2; 

    if (index_g > 2){
        uint16_t segment_g_md = (MY_HEAP[index_g-2] | (MY_HEAP[index_g-1] << 8)); 

        if (segment_g_md%2 == 0){
            uint16_t taille_totale_g = taille + segment_g_md + 4; 

                MY_HEAP[index+taille] = taille_totale_g & 0xFF;
                MY_HEAP[index+taille+1] = (taille_totale_g >> 8) & 0xFF;

                MY_HEAP[index_g-segment_g_md-4] = taille_totale_g & 0xFF;
                MY_HEAP[index_g-segment_g_md-3] = (taille_totale_g >> 8) & 0xFF;
        }
    }
}

void *my_malloc(size_t size){
    if (size>(SIZE_HEAP-4)){
        printf("La mémoire n'est pas assez grande pour l'allocation d'un tel bloc\n");
        return NULL;
    }

    // trouver l'index d'un espace adéquat dans lequel le segment peut se glisser
    uint16_t index = trouve_index1(size);
    if (index == CODE_ERREUR){
        printf("La mémoire n'a pas pu allouer un espace pour ce bloc\n");
        return NULL;
    }

    // padding de 2
    uint16_t taille;
    if (size%2 == 0) taille = size;
    else taille = size+1;

    // modifier l'emplacement et la valeur des blocs de MD du bloc vide restant (si nécessaire)
    uint16_t ancienne_md = lire_MD(index);
    if (ancienne_md > taille){
        uint16_t nouvelle_taille = ancienne_md - taille - 4;
        ecrire_MD(index+taille+4, nouvelle_taille);
        ecrire_MD(index+ancienne_md+2, nouvelle_taille);
    }

    // ajouter 1 pour le flag "occupé"
    uint16_t md = taille+1;

    // modifier les blocs de MD avant et après les blocs occupés
    ecrire_MD(index,md);
    ecrire_MD(index+taille+2,md);

    return &MY_HEAP[index];
}



//////////////////////////////////////
//   FONCTIONS UTILITAIRES          //
//   Déclarations supplémentaires   //
//////////////////////////////////////

uint16_t lire_MD (uint16_t index){
    uint16_t valeur = MY_HEAP[index-2] | (MY_HEAP[index-1] << 8);
    return valeur;
}

void ecrire_MD (uint16_t index, uint16_t valeur){
    MY_HEAP[index-2] = valeur & 0xFF;
    MY_HEAP[index-1] = (valeur >> 8) & 0xFF;
}

uint16_t trouve_index1 (size_t size){
    uint16_t index = 2;
    // lire la MD, si ça rentre retourner l'index, sinon chercher plus loin
    while (index <= SIZE_HEAP-(size+2)){
        uint16_t md_g = lire_MD(index);
        // si le bloc est vide et que sa taille est soit tout juste bonne, soit assez grande pour faire appraître un double bloc de MD
        if ((md_g%2 == 0) && ((md_g == size)||(md_g >= size+4))) return index;
        else index += (md_g +4 -(md_g%2)); 
    }
    // pas d'index adéquat trouvé
    return CODE_ERREUR;
}

void print_binary(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
}
void print_MD (uint16_t index){
    printf("Décimal : %u\n",lire_MD(index));

    printf("Binaire : ");
    print_binary(MY_HEAP[index-1]);
    printf(" ");
    print_binary(MY_HEAP[index-2]);
    printf("\n");
}


