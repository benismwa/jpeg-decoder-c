#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "bitreader.h"
#include "structures.h"
#include "huffman.h"
#include "mcu.h"

// Prédicteur DC global
int dc_pred[3] = {0, 0, 0};

// Tables Huffman préchargées (tu dois les remplir dans le main)
struct tab_huffman *huff_dc_tables[3];
struct tab_huffman *huff_ac_tables[3];
int dc_pred[3];
// Décodage du coefficient DC
int16_t decoder_DC(BitStream *stream, struct tab_huffman *huff_dc) {
    int magnitude = decode_huffman(huff_dc, stream);
    if (magnitude == 0) return 0;

    int16_t dc_value = lire_bits(stream, magnitude);
    if (dc_value < (1 << (magnitude - 1))) {
        dc_value -= (1 << magnitude) - 1;
    }
    return dc_value;
}

// Décodage des coefficients AC
void decoder_AC(BitStream *stream, struct tab_huffman *huff_ac, int16_t *bloc) {
    int i = 1;
    while (i < 64) {
        int symbol = decode_huffman(huff_ac, stream);

        if (symbol == 0x00) {  // End-of-Block
            while (i < 64) bloc[i++] = 0;
            break;
        } else if (symbol == 0xF0) {  // ZRL
            for (int j = 0; j < 16 && i < 64; j++) bloc[i++] = 0;
            continue;
        }

        int nb_zeros = (symbol >> 4) & 0xF;
        int magnitude = symbol & 0xF;

        while (nb_zeros-- && i < 64) bloc[i++] = 0;

        int ac_value = lire_bits(stream, magnitude);
        if (ac_value < (1 << (magnitude - 1))) {
            ac_value -= (1 << magnitude) - 1;
        }
        bloc[i++] = ac_value;
    }
}

// Décodage d’un bloc 8x8 complet (DC + AC)
void decoder_bloc_8x8_composante(BitStream *stream, int component_id, int16_t *bloc) {
    // Récupérer les bonnes tables Huffman pour cette composante
    struct tab_huffman *huff_dc = huff_dc_tables[component_id];
    struct tab_huffman *huff_ac = huff_ac_tables[component_id];

    if (!huff_dc || !huff_ac) {
        fprintf(stderr, "Erreur : Table Huffman absente pour la composante %d\n", component_id);
        return;
    }

    // DC
    int16_t diff = decoder_DC(stream, huff_dc);
    dc_pred[component_id] += diff;
    bloc[0] = dc_pred[component_id];

    // AC
    decoder_AC(stream, huff_ac, bloc);
}
