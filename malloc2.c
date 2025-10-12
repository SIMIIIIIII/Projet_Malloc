#include <stdint.h>
#include <stdio.h>


//////////////////////////////////
//   DECLARATIONS + "HEADERS"   //
//////////////////////////////////

#define CODE_ERREUR 65535 // 0xFFFF

uint8_t MY_HEAP[64000];

/**
 * La fonction init initialise la mémoire en plaçant un bloc de métadonnées au début et à la fin du tableau.
 */
void init();
/**
 * La fonction trouve_index1 parcourt le tableau depuis le début (index=0).
 * Elle saute d'un bloc de MD à l'autre pour trouver un endroit où le segment de taille 'size' peut se glisser.
 * Attention, le segment a besoin aussi de 4 blocs de MD en plus de sa taille 'size'.
 * Si elle a parcouru l'ensemble du tableau sans parvenir à trouver un endroit adéquat, elle retourne CODE_ERREUR.
 * 
 * Remarque : elle porte l'indice n°1 pour dire que c'est une version basique et qu'on pourra essayer d'autres version plus tard.
 */
uint16_t trouve_index1 (size_t size);
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
 * Fonctions utilitaires qui impriment les deux blocs de métadonnées qui se trouvent AVANT l'index indiqué.
 * Utiles pour débuguer, pas rendre dans inginious. 
 */
void print_MD (uint16_t index);
void print_binary(uint8_t byte);



///////////////////////////////////////
//   FONCTIONS CRUCIALES             //
//   à copier-coller dans Inginious  //
///////////////////////////////////////

void init(){
    //la mémoire contient 64000 blocs vides dont on retire 4 blocs de métadonnées
    uint16_t vide = 64000-(2*2);

    //métadonnées au début du bloc : 
    MY_HEAP[0] = vide & 0xFF;
    MY_HEAP[1] = (vide >> 8) & 0xFF;

    //métadonnées à la fin du bloc :
    MY_HEAP[63998] = vide & 0xFF;
    MY_HEAP[63999] = (vide >> 8) & 0xFF;

}

uint16_t trouve_index1 (size_t size){
    uint16_t index = 2;
    while (index <=64000){
        uint16_t md_gauche = MY_HEAP[index-2] | (MY_HEAP[index-1] << 8);
        if ((md_gauche%2 == 0) && (md_gauche+4 >= size)){
            return index;
        }
        else index += md_gauche+2;
    }
    return CODE_ERREUR;
}

void *my_malloc(size_t size){
    if (size>(64000-4)){
        printf("La mémoire n'est pas assez grande pour l'allocation d'un tel bloc");
        return NULL;
    }

    // trouver l'index d'un espace adéquat dans lequel le segment peut se glisser
    uint16_t index = trouve_index1(size);
    if (index == CODE_ERREUR){
        printf("La mémoire n'a pas pu allouer un espace pour ce bloc");
        return NULL;
    }

    // padding de 2
    uint16_t taille;
    if (size%2 == 0) taille = size;
    else taille = size+1;

    // modifier l'emplacement et la valeur des blocs de MD du bloc vide restant (si nécessaire)
    uint16_t ancienne_md = MY_HEAP[index-2] | (MY_HEAP[index-1] << 8);
    if (ancienne_md > taille){
        uint16_t nouvelle_taille = ancienne_md - taille - 4;

        MY_HEAP[index+taille+2] = nouvelle_taille & 0xFF;
        MY_HEAP[index+taille+3] = (nouvelle_taille >> 8) & 0xFF;

        MY_HEAP[index+ancienne_md] = nouvelle_taille & 0xFF;
        MY_HEAP[index+ancienne_md+1] = (nouvelle_taille >> 8) & 0xFF;
    }

    // ajouter 1 pour le flag "occupé"
    uint16_t md = taille+1;

    // modifier les blocs de MD avant et après les blocs occupés
    MY_HEAP[index-2] = md & 0xFF;
    MY_HEAP[index-1] = (md >> 8) & 0xFF;

    MY_HEAP[index+taille] = md & 0xFF;
    MY_HEAP[index+taille+1] = (md >> 8) & 0xFF;

    return &MY_HEAP[index];
}

void my_free(void *pointer){
    // récupérer l'index du tableau
    uint8_t *addr = (uint8_t*) pointer;
    uint16_t index = addr - MY_HEAP;

    // indiquer que le segment est désormais libre
    uint16_t taille = (MY_HEAP[index-2] | (MY_HEAP[index-1] << 8))-1;

    MY_HEAP[index-2] = taille & 0xFF;
    MY_HEAP[index-1] = (taille >> 8) & 0xFF;

    MY_HEAP[index+taille] = taille & 0xFF;
    MY_HEAP[index+taille+1] = (taille >> 8) & 0xFF;


    // vérifier si le segment à droite est libre aussi et, si oui, fusionner les deux segments
    uint16_t index_d = index + taille + 2;
    if (index_d < 64000){
        uint16_t segment_d_md = (MY_HEAP[index_d-2] | (MY_HEAP[index_d-1] << 8));

        if (segment_d_md%2 == 0){
            uint16_t taille_totale_d = taille + segment_d_md + 4;
            MY_HEAP[index-2] = taille_totale_d & 0xFF;
            MY_HEAP[index-1] = (taille_totale_d >> 8) & 0xFF;

            MY_HEAP[index+taille_totale_d] = taille & 0xFF;
            MY_HEAP[index+taille_totale_d+1] = (taille >> 8) & 0xFF;
        }
    }

    // vérifier si le segment à gauche est libre aussi et, si oui, fusionner les deux segments
    uint16_t index_g = index - 2;

    if (index_g > 2){
        uint16_t segment_g_md = (MY_HEAP[index_g-2] | (MY_HEAP[index_g-1] << 8));

        if (segment_g_md%2 == 0){
            uint16_t taille_totale_g = taille + segment_g_md + 4;

                MY_HEAP[index_g-2] = taille_totale_g & 0xFF;
                MY_HEAP[index_g-1] = (taille_totale_g >> 8) & 0xFF;

                MY_HEAP[index_g+taille_totale_g] = taille_totale_g & 0xFF;
                MY_HEAP[index_g+taille_totale_g+1] = (taille_totale_g >> 8) & 0xFF;
        }
    }
}



/////////////////////////////////////
//   FONCTIONS UTILITAIRES         //
//   pour les print et les tests   //
/////////////////////////////////////

void print_binary(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
}
void print_MD (uint16_t index){
    if ((index <2) || (index > 64000)){
        printf("Index incorrect\n");
        return;
    }

    uint16_t md = MY_HEAP[index-2] | (MY_HEAP[index-1] << 8);
    printf("Décimal : %u\n",md);

    printf("Binaire : ");
    print_binary(MY_HEAP[index-1]);
    printf(" ");
    print_binary(MY_HEAP[index-2]);
    printf("\n");
}

int main(void) {
    init();
    printf("Bloc vide avant\n");
    print_MD(2);

    printf("Bloc vide après\n");
    print_MD(64000);

    char *tab = (char*) my_malloc(3 * sizeof(char));

    printf("Bloc occupé avant\n");
    print_MD(2);

    printf("Bloc occupé après\n");
    print_MD(8);

    printf("Bloc vide restant avant\n");
    print_MD(10);

    printf("Bloc vide restant après\n");
    print_MD(64000);

    my_free(tab);

    return 0;
}

