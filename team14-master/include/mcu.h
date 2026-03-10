#ifndef MCU_H
#define MCU_H

#include <stdint.h>
#include "structures.h"
#include "bitreader.h"
#include "huffman.h"

typedef int16_t Block[64];
extern struct tab_huffman *huff_dc_tables[3];
extern struct tab_huffman *huff_ac_tables[3];
extern int dc_pred[3];

/*  Fonctions de dÃ©codage Huffman DC/AC */
int16_t decoder_DC(BitStream *stream, struct tab_huffman *huff_dc);
void decoder_AC(BitStream *stream, struct tab_huffman *huff_ac, int16_t *bloc);

/*  Fonction pour extraire un bloc MCU (8x8) */
void decoder_bloc_8x8_composante(BitStream *stream, int component_id, int16_t *bloc);
#endif  // MCU_H