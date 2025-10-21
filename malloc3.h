#ifndef MALLOC3_H
#define MALLOC3_H

#include <stdint.h>
#include <stdio.h>

#define SIZE_HEAP 64000
#define CODE_ERREUR 65535 
#define RESERVE 2
#define MOYEN 500

extern uint8_t MY_HEAP[SIZE_HEAP];


/**
 * La fonction init initialise la mémoire en plaçant un bloc de métadonnées au début et à la fin du tableau utilisable.
 * Elle tient compte de la réserve qui se place au début du tableau.
 * Cette réserve peut avoir une taille variable (y compris une taille nulle)!
 */
void init();

/**
 * La fonction my_malloc reçoit une taille 'size' en argument qui indique le nombre d'octets dont l'utilisateur a besoin.
 * Elle trouve un emplacement adéquat pour glisser le segment demandé, avec ses 4 blocs de métadonnées.
 * 
 * La recherche de l'emplacement adéquat est fait par une autre fonction (pas directement dans malloc!). 
 * Cette fonction retourne l'indice du tableau auquel commencent les données utilisateur (CAD à droite des deux blocs de MD)
 * L'adresse retournée à l'utilisateur par malloc est celle de cet endroit indiqué par trouve_index.
 * 
 * Elle modifie (si nécessaire) les métadonnées du bloc vide restant après l'allocation de mémoire.
 * Elle retourne à l'utilisateur un pointeur vers l'adresse du début de son segment de données utiles.
 * 
 * Si la taille du bloc demandé dépasse la taille de la mémoire ou si la mémoire est trop fragmentée pour placer le bloc, 
 * la fonction retourne NULL. 
 */
void *my_malloc(size_t size);

/**
 * La fonciton my_free reçoit un pointeur vers une zone du tableau. 
 * Elle modifie le bit de poids faible des métadonnées à gauche et à droite du segment pour indiquer qu'il est libre.
 * Elle vérifie à gauche et à droite du segment à libérer pour éventuellement fusionner des zones libres contigües.
 */
void my_free(void *pointer);

uint16_t my_find_asc(size_t size, uint16_t start);
uint16_t my_find_desc(size_t size, uint16_t start);

/**
 * Les fonctions lire_g et ecrire_g transforment les deux blocs de métadonnées situés à gauche de l'index donné
 * en une valeur décimale, et récriproquement.
 * Les fonctions lire_d et ecrire_d font la même chose avec les deux blocs situés à droite de l'index donné.
 */
uint16_t lire_g (uint16_t index);
void ecrire_g (uint16_t index, uint16_t valeur);
uint16_t lire_d(uint16_t index);
void ecrire_d(uint16_t index, uint16_t valeur);

/**
 * Les fonctions next_free trouvent l'indice du début des données utiles du prochain bloc libre 
 * respectivement à gauche ou à droite de l'indice donné.
 * Si l'index donné est le début d'un bloc libre, la fonction doit simplement retourner l'index
 */
uint16_t next_free_g(uint16_t index);
uint16_t next_free_d(uint16_t index);


#endif