#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include "malloc3.h" 

uint8_t MY_HEAP[SIZE_HEAP];

void test_lire_ecrire_g(){
    init();
    // zero
    ecrire_g(RESERVE+2,0);
    assert(lire_g(RESERVE+2) == 0);

    // petit nombre, uniquement sur l'octet de droite
    ecrire_g(RESERVE+4,8);
    assert(lire_g(RESERVE+4) == 8);

    // grand nombre, uniquement sur l'octet de gauche
    ecrire_g(RESERVE+6,64000);
    assert(lire_g(RESERVE+6) == 64000);

    // nombre impair
    ecrire_g(RESERVE+8,63999);
    assert(lire_g(RESERVE+8) == 63999);

    // fin de heap
    ecrire_g(SIZE_HEAP,2);
    assert(lire_g(SIZE_HEAP) == 2);
    printf("Test lire_ecrire_g est terminé!\n");
}

void test_lire_ecrire_d(){
    init();
    // zero
    ecrire_d(RESERVE,0);
    assert(lire_d(RESERVE) == 0);

    // petit nombre, uniquement sur l'octet de droite
    ecrire_d(RESERVE+2,8);
    assert(lire_d(RESERVE+2) == 8);

    // grand nombre, uniquement sur l'octet de gauche
    ecrire_d(RESERVE+4,64000);
    assert(lire_d(RESERVE+4) == 64000);

    // nombre impair
    ecrire_d(RESERVE+6,63999);
    assert(lire_d(RESERVE+6) == 63999);

    // fin de heap
    ecrire_d(SIZE_HEAP-2,2);
    assert(lire_d(SIZE_HEAP-2) == 2);
    printf("Test lire_ecrire_d est terminé!\n");
}

void test_init(){
    init();
    assert (lire_d(RESERVE) == SIZE_HEAP-RESERVE-4);
    assert (lire_g(SIZE_HEAP) == SIZE_HEAP-RESERVE-4);
    printf("Test init est terminé!\n");
}

int main(void) {
    printf("FONCTIONS UTILITAIRES :\n");
    test_lire_ecrire_g();
    test_lire_ecrire_d();
    printf("\n");

    //printf("\nTests de performance : \n");
    
    printf("Tous les tests sont terminés! 🎉\n");
    return 0;
}

//taper : gcc -Wall -Wextra -std=c11 test3.c malloc3.c -o test3
//et puis : ./test3