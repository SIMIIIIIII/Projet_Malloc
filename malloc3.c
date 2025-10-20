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
    // On conserve le padding de 2 mais alloue toujours un minimum de 4!
    // par exemple, si l'utilisateur demande 1,2,3 ou 4 byte, on lui donne 4. 
    // S'il demande 5 on lui donne 6, 7->8, 8->8, 9->10, etc.
    // utiliser my_free(index) sur le bloc vide résiduel à créer.    

    // ATTENTION : my_find ne fait plus gaffe à empêcher les petits morceaux dont on ne sait rien faire!
    // si my_find dit qu'il faut placer un bloc de 6 byte de données utiles (+4bytes de MD = 10 bytes) dans un bloc de moins de 18 bytes , il faut lui donner les 18 en un seul bloc!
    // pourquoi 18 bytes ? parce que le bloc vide minimum doit être de 4 byte (+4bytes de MD) pour stocker la liste chaînée de blocs vides. Sinon il ne sera plus jamais retrouvé.
}



uint16_t my_find_asc(size_t size, uint16_t start){
    uint16_t index = start;
    uint16_t md;

    uint16_t best_fit = CODE_ERREUR;
    uint16_t best_size = CODE_ERREUR;

    if (lire_g(index)%2!=0) index = nex_free_d(start);
    start = index; // mettre à jour le start sinon la condition du while pose problème

    do{
        md = lire_g(index);
        //autorise une marge de 10% pour le perfect fit
        if ((md == size) || ((md>=size)&&(md<=size+((size*10)/100)))) return index;
        //si pas de perfect-fit trouvé, on continue de mettre à jour le best-fit
        else if ((md >= size) && (md < best_size)){
            best_fit = index;
            best_size = md;
        }
        index = lire_d(index);

    } while (index != start); 
    return CODE_ERREUR;
}
uint16_t my_find_desc(size_t size, uint16_t start){
    uint16_t index = start;
    uint16_t md;

    uint16_t best_fit = CODE_ERREUR;
    uint16_t best_size = CODE_ERREUR;

    if (lire_g(index)%2!=0) index = nex_free_g(start);
    start = index; // mettre à jour le start sinon la condition du while pose problème

    do{
        md = lire_g(index);
        //autorise une marge de 10% pour le perfect fit
        if ((md == size) || ((md>=size)&&(md<=size+((size*10)/100)))) return index;
        else if ((md >= size) && (md < best_size)){
            best_fit = index;
            best_size = md;
        }
        index = lire_d(index+md);

    } while (index != start); 
    return CODE_ERREUR;
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