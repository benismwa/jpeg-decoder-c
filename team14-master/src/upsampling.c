#include "upsampling.h"
#include "structures.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void max_down_sampling(int nb_components, Parsed_file *jpeg, uint8_t *max_h, uint8_t *max_v) {
    Component *composants = jpeg->sof0_header.component;
    *max_h = 0;
    *max_v = 0;

    for (int i = 0; i < nb_components; i++) {
        if (composants[i].echant_horiz > *max_h) {
            *max_h = composants[i].echant_horiz;
        }
        if (composants[i].echant_vertic > *max_v) {
            *max_v = composants[i].echant_vertic;
        }
    }
}

void nbr_blocs_mcu(Parsed_file *jpeg, int nb_components, int *nb_blocs_mcu) {
    Component *composants = jpeg->sof0_header.component;
    *nb_blocs_mcu = 0;
    for (int i = 0; i < nb_components; i++) {
        *nb_blocs_mcu += composants[i].echant_horiz * composants[i].echant_vertic;
    }
}

void upsample_mcu_blocks_ycbcr(int8_t (*result)[64],
                                uint8_t (*bloc_image)[64],
                                int start_indices[3],
                                int h_factors[3], int v_factors[3],
                                int Hmax, int Vmax,
                                int total_blocs_mcu)
{
    int full_blocks = Hmax * Vmax;

    for (int c = 0; c < 3; c++) {
        int h1 = h_factors[0], v1 = v_factors[0];  // Y
        int h = h_factors[c], v = v_factors[c];

        int fact_h = h1 / h;
        int fact_v = v1 / v;

        int largeur = Hmax * 8;
        int hauteur = Vmax * 8;

        // Cas sans sous-échantillonnage : copie directe
        if (fact_h == 1 && fact_v == 1) {
            for (int i = 0; i < full_blocks; i++) {
                for (int j = 0; j < 64; j++) {
                    result[c * full_blocks + i][j] = bloc_image[start_indices[c] + i][j];
                }
            }
            continue;
        }

        // sinon on fait un up-sampling dans un tableau temporaire
        uint8_t *temp = calloc(largeur * hauteur, sizeof(uint8_t));
        if (!temp) {
            fprintf(stderr, "Erreur d'allocation mémoire temporaire.\n");
            exit(EXIT_FAILURE);
        }

        int num_blocs_h = h;
        int num_blocs_v = v;

        for (int bloc_v = 0; bloc_v < num_blocs_v; bloc_v++) {
            for (int bloc_h = 0; bloc_h < num_blocs_h; bloc_h++) {
                int bloc_index = start_indices[c] + bloc_v * num_blocs_h + bloc_h;

                if (bloc_index >= total_blocs_mcu) {
                    fprintf(stderr, "Erreur : bloc_index (%d) dépasse total_blocs_mcu (%d)\n", bloc_index, total_blocs_mcu);
                    free(temp);
                    exit(EXIT_FAILURE);
                }

                uint8_t *flat_block = bloc_image[bloc_index];

                for (int i = 0; i < 8; i++) {
                    for (int j = 0; j < 8; j++) {
                        uint8_t val = flat_block[i * 8 + j];

                        for (int x = 0; x < fact_v; x++) {
                            for (int y = 0; y < fact_h; y++) {
                                int new_i = bloc_v * 8 * fact_v + i * fact_v + x;
                                int new_j = bloc_h * 8 * fact_h + j * fact_h + y;
                                temp[new_i * largeur + new_j] = val;
                            }
                        }
                    }
                }
            }
        }

        // Reconstruction en blocs 8x8 dans `result`
        int idx = 0;
        for (int i = 0; i < hauteur; i += 8) {
            for (int j = 0; j < largeur; j += 8) {
                for (int y = 0; y < 8; y++) {
                    for (int x = 0; x < 8; x++) {
                        result[c * full_blocks + idx][y * 8 + x] = temp[(i + y) * largeur + (j + x)];
                    }
                }
                idx++;
            }
        }

        free(temp);
    }
}
