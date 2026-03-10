#include <stdio.h>
#include <stdint.h>

#include "bitreader.h"
#include <stdlib.h>
//	Structure qui garde : le fichier, un buffer (octet en cours) et combien de bits il reste à lire
//Initialise le lecteur de bits à partir d’un fichier JPEG

BitStream *init_bitstream(FILE *file) {  
    if (!file) {
        fprintf(stderr, " Erreur : Fichier invalide !\n");
        return NULL;
    }

    BitStream *bs = malloc(sizeof(BitStream));  // Alloue la mémoire pour un pointeur
    if (!bs) {
        fprintf(stderr, " Erreur : Échec d'allocation mémoire pour BitStream\n");
        return NULL;
    }

    bs->file = file;
    bs->bits_left = 0;
    bs->buffer = 0;

    return bs;  // Retourne un pointeur, pas une structure
}
//Lit un seul bit du fichier
int lire_bit(BitStream *bs) {
    if (!bs || !bs->file) return -1;
    if (bs->bits_left == 0) {
        bs->buffer = fgetc(bs->file);
        if (bs->buffer == 0xFF) {
            // skip 0x00 (byte stuffing)
            uint8_t next = fgetc(bs->file);
            if (next != 0x00) {
                fprintf(stderr, "Erreur : marqueur inattendu dans les données !\n");
                exit(1);
            }
        }
        bs->bits_left = 8;
    }
    bs->bits_left--;
    return (bs->buffer >> bs->bits_left) & 1;
}

uint16_t lire_bits(BitStream *bs, uint8_t n) {
    uint16_t val = 0;
    for (int i = 0; i < n; i++) {
        val = (val << 1) | lire_bit(bs);
    }
    return val;
}
void free_bitstream(BitStream *bs) {
    if (bs) {
        if (bs->file) {
            fclose(bs->file);
            bs->file = NULL;
        }
        free(bs);
        bs = NULL;  // Évite le double free
    }
}

    

