#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include "malloc.h"

//uint8_t MY_HEAP[SIZE_HEAP];

/*
void test_lire_ecrire(){
    init();
    // zero
    ecrire_MD(2,0);
    assert(lire_MD(2) == 0);

    // petit nombre, uniquement sur l'octet de droite
    ecrire_MD(4,8);
    assert(lire_MD(4) == 8);

    // grand nombre, uniquement sur l'octet de gauche
    ecrire_MD(6,64000);
    assert(lire_MD(6) == 64000);

    // nombre impair
    ecrire_MD(8,63999);
    assert(lire_MD(8) == 63999);

    // fin de heap
    ecrire_MD(SIZE_HEAP,2);
    assert(lire_MD(SIZE_HEAP));
}

void test_init(){
    init();
    assert (lire_MD(2) == SIZE_HEAP-4);
    assert (lire_MD(SIZE_HEAP) == SIZE_HEAP-4);
}

void test_malloc(){
    init();
    // vérifier la bonne réaction si on essaye d'allouer un bloc trop grand
    char *tab1 = (char*) my_malloc(SIZE_HEAP * sizeof(char));
    assert(tab1 == NULL);
    char *tab2 = (char*) my_malloc((SIZE_HEAP-2) * sizeof(char));
    assert(tab2 == NULL);

    // vérifier le padding de 2 et le flag à 1
    char *tab3 = (char*) my_malloc(3 * sizeof(char));
    assert(lire_MD(2) == 5);
    assert(lire_MD(8) == 5);
    assert(tab3 == (char*) &MY_HEAP[2]);

    // vérifier la resize de l'espace vide restant
    assert(lire_MD(10) == SIZE_HEAP-12);
    assert(lire_MD(SIZE_HEAP) == SIZE_HEAP-12);

    // vérifier la bonne réaction si la resize génère un bloc de taille zéro
    char *tab4 = (char*) my_malloc((SIZE_HEAP-16) * sizeof(char));
    assert(lire_MD(SIZE_HEAP-2) == 0);
    assert(lire_MD(SIZE_HEAP) == 0);
    assert(tab4 == (char*) &MY_HEAP[10]);
}

void test_malloc_full(){
    // ATTENTION : ces tests ne sont valables que si SIZE_HEAP = 64000

    init();
    // vérifier que la mémoire de 64000 peut bien accueillir 64 blocs de 996+4 bytes
    int count = 0;
    char *tab;
    do {
        tab = (char*) my_malloc(996 * sizeof(char));
        count += 1;
    } while (tab != NULL);
    //64 bloc de 996+4 alloués et puis un malloc qui a retourné null -> 65
    assert(count == 65);

    // vérifier que tous les blocs de MD à gauche et à droite des zones allouées ont la bonne valeur
    int index1 = 2;
    int index2 = 1000;
    while (index1 <= SIZE_HEAP){
        assert(lire_MD(index1) == 997);
        index1+=1000;
    }
    while (index2 <= SIZE_HEAP){
        assert(lire_MD(index2) == 997);
        index2+=1000;
    }
}

void test_free(){
    init();
    char *tab1 = (char*) my_malloc(4 * sizeof(char));
    char *tab2 = (char*) my_malloc(4 * sizeof(char));
    char *tab3 = (char*) my_malloc(4 * sizeof(char));
    char *tab4 = (char*) my_malloc(4 * sizeof(char));
    char *tab5 = (char*) my_malloc(4 * sizeof(char));
    char *tab6 = (char*) my_malloc(4 * sizeof(char));

    // vérifier la mise à zéro du flag, sans fusion
    my_free(tab2);
    assert(lire_MD(10) == 4);
    assert(lire_MD(16) == 4);

    // vérifier la fusion vers la droite
    my_free(tab1);
    assert(lire_MD(2) == 12);
    assert(lire_MD(16) == 12); 

    // vérifier la fusion vers la gauche
    my_free(tab4);
    my_free(tab5);
    assert(lire_MD(26) == 12);
    assert(lire_MD(40) == 12);

    // vérifier la double fusion
    my_free(tab3);
    assert(lire_MD(2) == 36);
    assert(lire_MD(40) == 36);

    // vérifier la remise à zéro lors du dernier free
    my_free(tab6);
    assert(lire_MD(2) == SIZE_HEAP-4);
    assert(lire_MD(SIZE_HEAP) == SIZE_HEAP-4);
}

void test_free_random(){
    init();
    void* tab[64];
    int indices[64];
    srand(time(NULL)); 

    // créer un tableau de 64 pointeurs vers 64 zones mémoires et un tableau de 64 indices mélangés aléatoirement
    for (int i=0; i<64; i++){
        tab[i] = my_malloc(996 * sizeof(char));
        assert(tab[i] != NULL);
        indices[i] = i;
    }
    for (int i = 64 - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = indices[i];
        indices[i] = indices[j];
        indices[j] = tmp;
    }

    // libérer tous les pointeurs dans un ordre aléatoire et vérifier que la mémoire revient à un état "vierge"
    for (int i=0; i<64; i++){
        my_free(tab[indices[i]]);
    }
    assert (lire_MD(2) == SIZE_HEAP-4);
    assert (lire_MD(SIZE_HEAP) == SIZE_HEAP-4);
}

void test_stress(){
    //remplir et vider le tas un certain nombre de fois, vérifier si la mémoire revient à l'état vierge
    for (int n=0; n<100; n++){
        srand(time(NULL)); 
        int size = rand() %10000+4;
        int nombre = SIZE_HEAP/size;

        void* tab[nombre];
        int indices[nombre];
        
        for (int i=0; i<nombre; i++){
            tab[i] = my_malloc(size * sizeof(char));
            assert(tab[i] != NULL);
            indices[i] = i;
        }

        srand(time(NULL)); 
        for (int i = nombre - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            int tmp = indices[i];
            indices[i] = indices[j];
            indices[j] = tmp;
        }

        for (int i=0; i<nombre; i++){
            my_free(tab[indices[i]]);
        }
        assert (lire_MD(2) == SIZE_HEAP-4);
        assert (lire_MD(SIZE_HEAP) == SIZE_HEAP-4);

    }


}

void test_trouve1(){
    init();
    char *tab1 = (char*) my_malloc(4 * sizeof(char));  
    char *tab2 = (char*) my_malloc(4 * sizeof(char)); 
    char *tab3 = (char*) my_malloc(4 * sizeof(char)); 
    my_free(tab2);

    // la fonction doit ignorer le segment trop petit!
    char *tab4 = (char*) my_malloc(5 * sizeof(char));
    assert(tab4 == (char*) &MY_HEAP[26]);

    // la fonction doit faire rentrer le segment dans une taille tout juste assez grande
    int * nb1 = (int*) my_malloc(sizeof(int));
    assert(nb1 == (int*) &MY_HEAP[10]);

    init();
    char *tab5 = (char*) my_malloc(4 * sizeof(char));  
    char *tab6 = (char*) my_malloc(4 * sizeof(char)); 
    my_free(tab5);

    // la fonction doit ignorer le segment assez grand mais qui ne permet pas de faire des blocs MD valides
    char *tab7 = (char*) my_malloc(2 * sizeof(char));
    assert(tab7 == (char*) &MY_HEAP[18]);

    // le trou proposé est assez grand et permet de glisser des métadonnées adéquates
    // vérifier que la fonction free transforme adéquatement les MD du nouveau bloc
    my_free(tab6);
    char *tab8 = (char*) my_malloc(4 * sizeof(char)); 
    assert(tab8 = (char*) &MY_HEAP[2]);
    assert(lire_MD(10) == 4);
}

void test_trouve2(){
    init();
    char *tab1 = (char*) my_malloc(4 * sizeof(char));  
    char *tab2 = (char*) my_malloc(4 * sizeof(char)); 
    char *tab3 = (char*) my_malloc(4 * sizeof(char)); 
    my_free(tab2);

    // la fonction doit ignorer le segment trop petit!
    char *tab4 = (char*) my_malloc(6 * sizeof(char));
    assert(tab4 == (char*) &MY_HEAP[26]);

    // la fonction doit faire rentrer le segment dans une taille tout juste assez grande
    char *tab5 = (char*) my_malloc(4 * sizeof(char));
    assert(tab5 == (char*) &MY_HEAP[10]);

    // la fonction doit chercher le perfect_fit et ne pas prendre le premier fit qui passe
    char *tab6 = (char*) my_malloc(4 * sizeof(char));
    char *tab7 = (char*) my_malloc(4 * sizeof(char));
    char *tab8 = (char*) my_malloc(4 * sizeof(char));
    my_free(tab4);
    my_free(tab3);
    my_free(tab7);

    char *tab9 = (char*) my_malloc(4 * sizeof(char));
    assert(tab9 == (char*) &MY_HEAP[44]);

    // faute de perfect fit, la fonction doit prendre le best_fit
    char *tab10 = (char*) my_malloc(4 * sizeof(char));
    assert(tab10 == (char*) &MY_HEAP[18]);
}

void etat_memoire(){
    uint16_t blocs_libres = 0; // nombre total de blocs libres rencontrés
    uint16_t blocs_occupes = 0; // nombre total de blocs occupés rencontrés
    uint16_t max_libre = 0; // taille du plus grand bloc libre (hors métadonnées)
    uint16_t total_libre = 0; // quantité totale de bytes libres (hors métadonnées)
    double indice_fragmentation;

    uint16_t indice = 2;
    while (indice <= 64000){
        uint16_t md_g = lire_MD(indice);
        if (md_g %2 == 0){
            blocs_libres ++;
            total_libre += md_g;
            if (md_g > max_libre) max_libre = md_g;
        }
        else{
            blocs_occupes ++;
        }
        indice += (md_g +4 -(md_g%2));
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

*/

void test_random_steps(){
    init();

    //déterminer un range de taille pour les données à allouer
    uint16_t sizemax = 1000;
    uint16_t sizemin = 500;
    uint16_t length = SIZE_HEAP/sizemin;

    //compteurs de résultats : 
    int nulls = 0;
    int mallocs = 0;
    int frees = 0;

    // créer un tableau qui contient tous les pointeurs vers les zones allouées
    void* tab[length];
    for (uint16_t i=0; i<length; i++){
        tab[i] = NULL;
    }

    for (int step=0; step<1000; step++){
        uint16_t i = rand() % length; // choisir une case aléatoire
        if (tab[i]==NULL){
            size_t size = sizemin + rand() % (sizemax - sizemin + 1);
            tab[i] = my_malloc(size);
            if (tab[i]==NULL){
                nulls++;
            }
            
            else {
                mallocs++;
            }
        }
        else {
            my_free(tab[i]);
            tab[i] = NULL;
            frees++;
        }
    }
  
    printf("Résultats du test random_steps : \n");
    printf("Nombre de mallocs : %d\n", mallocs);
    printf("Nombre de frees : %d\n", frees);
    printf("Nombre de nuls : %d\n", nulls);
    printf("\n");
    etat_memoire();
    printf("\n");
}

void test_random_time(){
    init();

    //déterminer un range de taille pour les données à allouer
    uint16_t sizemax = 1000;
    uint16_t sizemin = 500;
    uint16_t length = SIZE_HEAP/sizemin;

    //compteurs de résultats : 
    int nulls = 0;
    int mallocs = 0;
    int frees = 0;

    // créer un tableau qui contient tous les pointeurs vers les zones allouées
    void* tab[length];
    for (uint16_t i=0; i<length; i++){
        tab[i] = NULL;
    }

    struct timeval start, now;
    gettimeofday(&start, NULL);

    double elapsed = 0.0;
    while (elapsed < 1.0) {
        uint16_t i = rand() % length; // choisir une case aléatoire
        if (tab[i]==NULL){
            size_t size = sizemin + rand() % (sizemax - sizemin + 1);
            tab[i] = my_malloc(size);
            if (tab[i]==NULL){
                nulls++;
            }
            
            else {
                mallocs++;
            }
        }
        else {
            my_free(tab[i]);
            tab[i] = NULL;
            frees++;
        }
        gettimeofday(&now, NULL);
        elapsed = (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1e6;
    }
    
    
    printf("Résultats du test random_time : \n");
    printf("Nombre de mallocs : %d\n", mallocs);
    printf("Nombre de frees : %d\n", frees);
    printf("Nombre de nuls : %d\n", nulls);
    printf("Nombre de elapse : %d\n", elapsed);
    printf("\n");
    etat_memoire();
    printf("\n");
}

int main(void) {
    //test_lire_ecrire();
    //test_init();
    //test_malloc();
    //test_malloc_full(); 
    //test_free();
    //test_free_random(); 
    //test_stress();
    //test_trouve1();
    //test_trouve2();
    test_random_steps();
    test_random_time();
    
    printf("Tests terminés! 🎉\n");
    return 0;
}

//taper : gcc -Wall -Wextra -std=c11 test2.c malloc2.c -o test2
//et puis : ./test2
