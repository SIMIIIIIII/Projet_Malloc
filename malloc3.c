#include <stdint.h>
#include <stdio.h>
#include "malloc3.h"

///////////////////////////////////////
//   FONCTIONS CRUCIALES             //
//   à copier-coller dans Inginious  //
///////////////////////////////////////

void init(){
    uint16_t valeur = SIZE_HEAP-RESERVE-4;

    //mettre les deux blocs de MD à gauche et à droite de la grande zone vide
    ecrire_d(RESERVE,valeur); 
    ecrire_g(SIZE_HEAP,valeur);

    //écrire dans les données utilisateur que le prochain bloc vide c'est lui-même
    ecrire_d(RESERVE+2,RESERVE);
    ecrire_g(SIZE_HEAP-2,RESERVE);
}

void my_free(void *pointer){
    //récupérer l'index du tableau
    uint8_t *addr = (uint8_t*) pointer;
    uint16_t index = addr - MY_HEAP; 

    // indiquer que le segment est désormais libre
    uint16_t taille = lire_g(index)-1;         

    ecrire_g(index,taille);
    ecrire_d(index+taille,taille);

    // vérifier si le segment à droite est libre aussi et, si oui, fusionner les deux segments
    uint16_t index_d = index+taille+4;
    if (index_d <= SIZE_HEAP-6){
        uint16_t md_d = lire_g(index_d); 

        if (md_d%2 == 0){
            taille = taille + md_d + 4; 
            ecrire_g(index,taille);
            ecrire_d(index+taille,taille); 
        }
    }

    // vérifier si le segment à gauche est libre aussi et, si oui, fusionner les deux segments
    if (index-2 > RESERVE+2){
        uint16_t segment_g_md = lire_g(index-2); 
        if (segment_g_md%2 == 0){
            uint16_t index = index-4-segment_g_md;
            uint16_t taille = taille + segment_g_md + 4; 

            ecrire_g(index,taille);
            ecrire_d(index+taille,taille);
        }
    }
    
    // ajouter dans les données utilisateur l'emplacement du prochain bloc vide à droite
    uint16_t next = next_free_d(index);
    if (next != CODE_ERREUR){
        ecrire_d(index,next);
    }
    else printf("Echec d'écriture");
    // pareil pour la gauche
    next = next_free_g(index);
    if (next != CODE_ERREUR){
        ecrire_g(index+taille,next);
    }
    else printf("Echec d'écriture");
}

void *my_malloc(size_t size){
    //TODO
}



uint16_t my_find_asc(size_t size, uint16_t start){
    uint16_t index = start;
    uint16_t md;

    uint16_t best_fit = CODE_ERREUR;
    uint16_t best_size = CODE_ERREUR;

    if (lire_g(index)%2!=0) index = nex_free_d(start);
    start = index;

    do{
        md = lire_g(index);
        if ((md == size) || ((md>=size)&&(md<=size+((size*15)/100)))) return index;
        else if ((md >= size) && (md < best_size)){
            best_fit = index;
            best_size = md;
        }
        index = lire_d(index);

    } while (index != start); 
    return CODE_ERREUR;
}
uint16_t my_find_desc(size_t size, uint16_t start){
    //TODO
    return 0;
}


//////////////////////////////////////
//   FONCTIONS UTILITAIRES          //
//   Déclarations supplémentaires   //
//////////////////////////////////////

uint16_t lire_g (uint16_t index){
    uint16_t valeur = MY_HEAP[index-2] | (MY_HEAP[index-1] << 8);
    return valeur;
}

void ecrire_g (uint16_t index, uint16_t valeur){
    MY_HEAP[index-2] = valeur & 0xFF;
    MY_HEAP[index-1] = (valeur >> 8) & 0xFF;
}

uint16_t lire_d(uint16_t index){
    uint16_t valeur = MY_HEAP[index + 1] | (MY_HEAP[index + 2] << 8);
    return valeur;
}

void ecrire_d(uint16_t index, uint16_t valeur){
    MY_HEAP[index + 1] = valeur & 0xFF;
    MY_HEAP[index + 2] = (valeur >> 8) & 0xFF;
}

uint16_t next_free_d(uint16_t index){
    uint16_t find = index;
    uint16_t md = lire_g(find);
    do {
        find += md+4-(md%2);
        if (find>SIZE_HEAP-6) find = RESERVE+2;
        md = lire_g(find);
        if (md%2 == 0) return find;
        } while (index != find);
    return CODE_ERREUR;
}

uint16_t next_free_g(uint16_t index){
    uint16_t find = index;
    uint16_t md;
    do {
        find = find-2;
        if (find<=RESERVE) find = SIZE_HEAP;
        md = lire_g(find);
        find = find-(md-(md%2))-2;
        if (md%2==0) return find;
    } while (find!= index);
    return CODE_ERREUR;

}