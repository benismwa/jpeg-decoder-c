#include "../include/huffman.h"
#include "../include/bitreader.h"
#include "../include/structures.h"
#include <stdlib.h>
#include <stdio.h>

/* Création d'un nouveau nœud Huffman */
struct tab_huffman *tab_huffman_nouveau_noeud() {
    struct tab_huffman *n = malloc(sizeof(struct tab_huffman));
    if (!n) {
        fprintf(stderr, "Erreur : Allocation de mémoire échouée pour Huffman !\n");
        return NULL;
    }
    n->fils = calloc(2, sizeof(struct tab_huffman *)); // Initialiser les fils
    n->valeur = 0;
    n->set = false;
    return n;
}

/*  Création d'un arbre Huffman vide */
struct tab_huffman *tab_huffman_vide() {
    struct tab_huffman *racine = tab_huffman_nouveau_noeud();
    if (!racine) {
        fprintf(stderr, "Erreur : Impossible d'allouer l'arbre Huffman !\n");
        return NULL;
    }
    return racine;
}

/*  Construction de l'arbre Huffman */
void huffman_construire_arbre(struct tab_huffman *racine, uint8_t *offset, uint8_t *valeurs) {
    if (!racine || !offset || !valeurs) {
        fprintf(stderr, "Erreur : Données Huffman invalides !\n");
        return;
    }

    uint16_t code = 0;
    int k = 0;

    for (int longueur = 1; longueur <= 16; ++longueur) {
        int nb = offset[longueur - 1];
        for (int i = 0; i < nb; ++i) {
            uint8_t symbole = valeurs[k++];
            inserer_code(racine, symbole, code, longueur);
            code++;
        }
        code <<= 1;
    }
}

/* Insertion d'un code Huffman */
void inserer_code(struct tab_huffman *racine, uint8_t symbole, uint16_t code, int longueur) {
    struct tab_huffman *courant = racine;
    for (int i = longueur - 1; i >= 0; --i) {
        int bit = (code >> i) & 1;

        if (!courant->fils) {
            courant->fils = calloc(2, sizeof(struct tab_huffman *));
        }
        if (!courant->fils[bit]) {
            courant->fils[bit] = tab_huffman_nouveau_noeud();
        }

        courant = courant->fils[bit];
    }

    courant->valeur = symbole;
    courant->set = true;
}

/* Décodage Huffman */
int decode_huffman(struct tab_huffman *abr, BitStream *stream) {
    if (!abr) {
        fprintf(stderr, "Erreur : Arbre Huffman NULL !\n");
        return -1;
    }

    struct tab_huffman *courant = abr;
    while (1) {
        if (courant->set) return courant->valeur;

        int bit = lire_bit(stream);
        if (bit == -1) {
            fprintf(stderr, "Erreur : Flux de bits corrompu !\n");
            return -1;
        }

        if (!courant->fils || !courant->fils[bit]) {
            fprintf(stderr, "Erreur : Impossible de suivre le chemin Huffman !\n");
            return -1;
        }

        courant = courant->fils[bit];
    }
}

/*Récupération d'une table Huffman */
struct tab_huffman *get_huffman_table(int index, int type, Parsed_file *jpeg) {
    if (index < 0 || index > 3 || type < 0 || type > 1) {
        fprintf(stderr, "Erreur : Indice Huffman hors limites (index=%d, type=%d)\n", index, type);
        return NULL;
    }

    uint8_t *lengths = jpeg->huffman_tables[index][type].lengths;
    uint8_t *symbols = jpeg->huffman_tables[index][type].symbols;
    
    if (!lengths || !symbols) {
        fprintf(stderr, "Erreur : Tables Huffman vides !\n");
        return NULL;
    }

    struct tab_huffman *racine = tab_huffman_vide();
    if (!racine) {
        fprintf(stderr, "Erreur : Impossible d'allouer l'arbre Huffman !\n");
        return NULL;
    }

    huffman_construire_arbre(racine, lengths, symbols);
    return racine;
}

int init_huffman_tables(Parsed_file *parsed, int nb_composantes,
    tab_huffman **huff_dc_tables,
    tab_huffman **huff_ac_tables) {
        
    for (int i = 0; i < nb_composantes; i++) {
        int dc_id = parsed->sos_header.composantes[i].huffman_DC;
        int ac_id = parsed->sos_header.composantes[i].huffman_AC;

        huff_dc_tables[i] = get_huffman_table(dc_id, 0, parsed);
        huff_ac_tables[i] = get_huffman_table(ac_id, 1, parsed);

        if (!huff_dc_tables[i] || !huff_ac_tables[i]) {
            fprintf(stderr, "Erreur : table Huffman non trouvée pour la composante %d\n", i);
            return -1;
        }
    }
    return 0;
}
/*Libération de l'arbre Huffman */
void tab_huffman_free(struct tab_huffman *racine) {
    if (!racine) return;

    if (racine->fils) {
        if (racine->fils[0]) {
            tab_huffman_free(racine->fils[0]);
            racine->fils[0] = NULL;
        }
        if (racine->fils[1]) {
            tab_huffman_free(racine->fils[1]);
            racine->fils[1] = NULL;
        }
        free(racine->fils);
        racine->fils = NULL;
    }

    free(racine);
}
