#ifndef MALLOC2_H
#define MALLOC2_H

#include <stdint.h>
#include <stdio.h>

#define SIZE_HEAP 64000
#define CODE_ERREUR 65535 

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

/**
 * Les fonctions lire_MD et ecrire_md transforment les deux blocs de métadonnées situées à gauche de l'index donné
 * en une valeur décimale, et récriproquement.
 */
uint16_t lire_MD (uint16_t index);
void ecrire_MD (uint16_t index, uint16_t valeur);

/**
 * La fonction trouve_index1 parcourt le tableau depuis le début (index=0).
 * Elle saute d'un bloc de MD à l'autre pour trouver un endroit où le segment de taille 'size' peut se glisser.
 * Si elle a parcouru l'ensemble du tableau sans parvenir à trouver un endroit adéquat, elle retourne CODE_ERREUR.
 */
uint16_t trouve_index1 (size_t size);

/**
 * La fonction trouve_index2 est une version plus performante de trouve_index1.
 * La fonction parcourt le tableau depuis le début (index=2) et retient le best_fit qu'elle a rencontré.
 * Si elle trouve un perfect_fit, elle se glisse dedans immédiatement. 
 * Si elle a parcouru l'ensemble de la mémoire sans trouver de perfect_fit, elle retourne au best_fit qu'elle a trouvé.
 * Si elle a parcouru l'ensemble du tableau sans parvenir à trouver un endroit adéquat, elle retourne CODE_ERREUR.
 */
uint16_t trouve_index2 (size_t size);

/**
 * La fonction trouve_index3 est une version plus rapide mais moins judicieuse en terme de mémoire.
 * La fonction parcourt le tableau depuis le début (index=2) et retient le best_fit qu'elle a rencontré.
 * Si elle trouve un almost_perfect_fit, elle se glisse dedans immédiatement. 
 * Si elle a parcouru l'ensemble de la mémoire sans trouver de perfect_fit, elle retourne au best_fit qu'elle a trouvé.
 * Si elle a parcouru l'ensemble du tableau sans parvenir à trouver un endroit adéquat, elle retourne CODE_ERREUR.
 */
uint16_t trouve_index3 (size_t size);

/**
 * Fonctions utilitaires qui impriment les deux blocs de métadonnées qui se trouvent AVANT l'index indiqué.
 * Utiles pour débuguer, pas rendre dans inginious. 
 */
void print_MD (uint16_t index);
void print_binary(uint8_t byte);

#endif