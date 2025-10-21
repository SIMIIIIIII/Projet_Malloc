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

void test_lire_ecrire(){
    init();
    //mêmes tests qu'avant, mais mélanger droite et gauche!

    // zero
    ecrire_g(RESERVE+2,0);
    assert(lire_g(RESERVE+2) == lire_d(RESERVE));

    // petit nombre, uniquement sur l'octet de droite
    ecrire_d(RESERVE+2,8);
    assert(lire_d(RESERVE+2) == lire_g(RESERVE+4));

    // grand nombre, uniquement sur l'octet de gauche
    ecrire_g(RESERVE+6,64000);
    assert(lire_g(RESERVE+6) == lire_d(RESERVE+4));

    // nombre impair
    ecrire_d(RESERVE+6,63999);
    assert(lire_d(RESERVE+6) == lire_g(RESERVE+8));

    // fin de heap
    ecrire_g(SIZE_HEAP,2);
    assert(lire_g(SIZE_HEAP) == lire_d(SIZE_HEAP-2));

    printf("Test lire_ecrire est terminé!\n");

}

void test_init(){
    init();
    assert (lire_d(RESERVE) == SIZE_HEAP-RESERVE-4);
    assert (lire_g(SIZE_HEAP) == SIZE_HEAP-RESERVE-4);
    printf("Test init est terminé!\n");
}

void test_next_free_d(){
    init();
    
    // le bloc sur lequel on est doit être considéré
    ecrire_d(RESERVE,4);
    assert(next_free_d(RESERVE+2 == RESERVE+2));

    // le bloc libre suivant est juste à droite
    ecrire_d(RESERVE,5);
    ecrire_d(RESERVE+8,4);
    assert(next_free_d(RESERVE+2) == RESERVE+10);

    // le bloc libre suivant est plus loin (tailles identiques)
    ecrire_d(RESERVE+8,5);
    ecrire_d(RESERVE+16,4);
    assert(next_free_d(RESERVE+2) == RESERVE+18);

    // le bloc libre suivant est plus loin (tailles différentes)
    ecrire_d(RESERVE+16,7);
    ecrire_d(RESERVE+26,4);
    assert(next_free_d(RESERVE+2) == RESERVE+28);

    // le bloc libre suivant nécessite de retourner au début du tas(le premier bloc est libre)
    ecrire_d(RESERVE,4);
    ecrire_d(RESERVE+8,5);
    ecrire_d(RESERVE+16,SIZE_HEAP-RESERVE-15);
    assert(next_free_d(RESERVE+10)== RESERVE+2);
    
    // le bloc libre suivant nécessite de retourner au début du tas (le premier bloc n'est pas libre)
    ecrire_d(RESERVE,5);
    ecrire_d(RESERVE+8,4);
    assert(next_free_d(RESERVE+18) == RESERVE+10);

    // il n'y a pas de bloc libre, la fonction doit retourner un code d'erreur quel que soit l'endroit où elle démarre
    ecrire_d(RESERVE,5);
    ecrire_d(RESERVE+8,5);
    ecrire_d(RESERVE+16,SIZE_HEAP-RESERVE-15-4);
    assert(next_free_d(RESERVE+2) == CODE_ERREUR);
    assert(next_free_d(RESERVE+10) == CODE_ERREUR);
    assert(next_free_d(RESERVE+18) == CODE_ERREUR);

    printf("Test next_free_d terminé!\n");
}

void test_next_free_g(){
    init();
    
    // le bloc sur lequel on est doit être considéré 
    ecrire_d(RESERVE,4);
    assert(next_free_g(RESERVE+2) == RESERVE+2);

    // le bloc libre suivant est juste à gauche
    ecrire_d(RESERVE,4);
    ecrire_d(RESERVE+6,4);
    ecrire_d(RESERVE+8,5);
    assert(next_free_g(RESERVE+10) == RESERVE+2);

    // le bloc libre suivant est plus loin (tailles identiques)
    ecrire_d(RESERVE+14,5);
    ecrire_d(RESERVE+16,5);
    assert(next_free_g(RESERVE+18) == RESERVE+2);

    // le bloc libre suivant est plus loin (tailles différentes)
    ecrire_d(RESERVE+22,5);
    ecrire_d(RESERVE+24,7);
    ecrire_d(RESERVE+32,7);
    ecrire_d(RESERVE+34,5);
    assert(next_free_g(RESERVE+36) == RESERVE+2);

    // le bloc libre suivant nécessite de retourner à la fin du tas(le dernier bloc est libre)
    ecrire_d(RESERVE,5);
    ecrire_d(RESERVE+6,5);
    ecrire_g(SIZE_HEAP,4);
    ecrire_g(SIZE_HEAP-6,4);
    assert(next_free_g(RESERVE+10) == SIZE_HEAP-6);
    
    // le bloc libre suivant nécessite de retourner à la fin du tas (le dernier bloc n'est pas libre)
    ecrire_g(SIZE_HEAP,5);
    ecrire_g(SIZE_HEAP-6,5);
    ecrire_g(SIZE_HEAP-8,4);
    ecrire_g(SIZE_HEAP-14,4);
    assert(next_free_g(RESERVE+10) == SIZE_HEAP-14);

    // il n'y a pas de bloc libre, la fonction doit retourner un code d'erreur quel que soit l'endroit où elle démarre
    ecrire_d(RESERVE,5);
    ecrire_d(RESERVE+6,5);
    ecrire_d(RESERVE+8,5);
    ecrire_d(RESERVE+14,5);
    ecrire_d(RESERVE+16,SIZE_HEAP-RESERVE-15-4);
    ecrire_g(SIZE_HEAP,SIZE_HEAP-RESERVE-15-4);
    assert(next_free_g(RESERVE+2) == CODE_ERREUR);
    assert(next_free_g(RESERVE+10) == CODE_ERREUR);
    assert(next_free_g(RESERVE+18) == CODE_ERREUR);

    printf("Test next_free_g terminé!\n");
}

int main(void) {
    printf("\nFONCTIONS UTILITAIRES :\n");
    test_lire_ecrire_g();
    test_lire_ecrire_d();
    test_lire_ecrire();
    test_next_free_d();
    test_next_free_g();
    
    printf("\nFONCTIONS CRUCIALES : \n");
    test_init();

    //printf("\nTests de performance : \n");
    
    printf("Tous les tests sont terminés! 🎉\n");
    return 0;
}

//taper : gcc -Wall -Wextra -std=c11 test3.c malloc3.c -o test3
//et puis : ./test3