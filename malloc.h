#ifndef MALLOC3_H
#define MALLOC3_H

#include <stdint.h>
#include <stdio.h>

#define SIZE_HEAP 64000
extern uint8_t MY_HEAP[SIZE_HEAP];


/**
 * La fonction init initialise la mémoire en plaçant un bloc de métadonnées au début et à la fin du tableau.
 */
void init();

/**
 * La fonction my_malloc reçoit une taille 'size' en argument qui indique le nombre d'octets dont l'utilisateur a besoin.
 * Elle trouve un emplacement adéquat pour glisser le segment demandé, avec ses 4 blocs de métadonnées.
 * Elle modifie (si nécessaire) les métadonnées du bloc vide restant après l'allocation de mémoire.
 * Elle retourne à l'utilisateur un pointeur vers l'adresse du début de son segment de données utiles.
 * 
 * Si la taille du bloc demandé dépasse la taille de la mémoire ou si la mémoire est trop fragmentée pour placer le bloc, 
 * la fonction retourne NULL. * 
 */
void *my_malloc(size_t size);

/**
 * La fonciton my_free reçoit un pointeur vers une zone du tableau. 
 * Elle modifie le bit de poids faible du segment pour indiquer que ce segment est libre.
 * Elle vérifie à gauche et à droite du segment à libérer pour éventuellement fusionner des zones libres.
 */
void my_free(void *pointer);

void etat_memoire();

#endif