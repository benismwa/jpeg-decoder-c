#ifndef HUFFMAN_H
#define HUFFMAN_H
#include "bitreader.h"
#include <stdint.h>
#include <stdbool.h>
#include "structures.h" // Ajoute l'inclusion de structures.h
typedef struct tab_huffman {
    struct tab_huffman **fils;
    uint8_t valeur;
    bool set;
}tab_huffman;

tab_huffman *tab_huffman_nouveau_noeud();
tab_huffman *tab_huffman_vide();
tab_huffman *get_huffman_table(int index, int type,Parsed_file *jpeg);

void huffman_construire_arbre( tab_huffman *racine, uint8_t *offset, uint8_t *valeurs);
void inserer_code(tab_huffman *racine, uint8_t symbols, uint16_t code, int longueur);
int decode_huffman( tab_huffman *abr, BitStream *stream);
int init_huffman_tables(Parsed_file *parsed, int nb_composantes, tab_huffman **huff_dc_tables,
    tab_huffman **huff_ac_tables);

void tab_huffman_free(struct tab_huffman *racine);
#endif // HUFFMAN_H
