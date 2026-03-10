#ifndef BITREADER_H
#define BITREADER_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
    FILE *file;
    uint8_t buffer;  // Octet courant
    int bits_left;   // Nombre de bits restants dans l'octet buffer
} BitStream;

BitStream *init_bitstream(FILE *file);
int lire_bit(BitStream *bs);  // Harmonisation avec fgetc() (renvoie un int)
uint16_t lire_bits(BitStream *bs, uint8_t n);  // Harmonisation avec les autres appels
void free_bitstream(BitStream *bs);
#endif  // BITREADER_H
