#include "../include/huffman.h"
#include "../include/bitreader.h"
#include "../include/structures.h"
#include "../include/mcu.h"
#include "../include/ecriture.h"
#include "../include/parser.h"
#include "../include/quantification.h"
#include "../include/zigzag.h"
#include "../include/ycbcr2rgb.h"
#include "../include/upsampling.h"
#include "../include/idct.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    char output_filename[256];

    // Générer le nom du fichier de sortie .ppm
    generation_nom_fichier(filename,output_filename,sizeof(output_filename));
    
    FILE *jpeg_file = fopen(filename, "rb");
    if (!jpeg_file) {
        fprintf(stderr, "Erreur : impossible d’ouvrir le fichier %s\n", filename);
        return EXIT_FAILURE;
    }

    Parsed_file parsed = {0};
    // Lecture des entêtes JPEG (SOI, SOF0, DQT, DHT, SOS, etc.)
    lire_entete(jpeg_file, &parsed); 

    // Initialiser le bitstream pour lecture des données compressées
    BitStream *br = init_bitstream(jpeg_file);
    if (!br) {
        fprintf(stderr, "Erreur : init_bitstream échouée\n");
        return EXIT_FAILURE;
    }

    const int width = parsed.image_largeur;
    const int height = parsed.image_hauteur;
    const int nb_composantes = parsed.sof0_header.nbr_composantes;

    // Calcul des facteurs d'échantillonnage maximum pour upsampling
    uint8_t Hmax, Vmax;
    max_down_sampling(nb_composantes, &parsed, &Hmax, &Vmax);
    const int H_V_max = Hmax * Vmax;

    // Calcul du nombre de blocs 8x8 horizontaux et verticaux
    const int blocs_par_ligne = (width + 8 * Hmax - 1) / (8 * Hmax);
    const int blocs_par_colonne = (height + 8 * Vmax - 1) / (8 * Vmax);
    const int nb_blocs = blocs_par_ligne * blocs_par_colonne;

    int total_blocs_mcu;
    nbr_blocs_mcu(&parsed, nb_composantes, &total_blocs_mcu);

    // Initialiser les tables de Huffman pour chaque composante
    if (init_huffman_tables(&parsed, nb_composantes, huff_dc_tables, huff_ac_tables) < 0) {
        // gestion d’erreur, cleanup si besoin
        fprintf(stderr, "Erreur lors de l'initialisation des tables Huffman.\n");
        return EXIT_FAILURE;
    }

    // Allocation mémoire pour le traitement
    int16_t (*mcu)[64] = malloc(total_blocs_mcu * sizeof(*mcu));
    int16_t (*matrix)[8][8] = malloc(total_blocs_mcu * sizeof(*matrix));
    uint8_t (*bloc_image)[64] = malloc(total_blocs_mcu * sizeof(*bloc_image));
    int16_t (*iquant)[64] = malloc(total_blocs_mcu * sizeof(*iquant));
    int8_t (*upsampled_blocks)[64] = malloc(3 * H_V_max * sizeof(*upsampled_blocks));
    int16_t *image_finale = malloc(nb_blocs * H_V_max * nb_composantes * 64 * sizeof(int16_t));

    uint8_t bloc_ycbcr[H_V_max][3][64];
    uint8_t rgb[H_V_max][3][64];
    uint8_t (*converted_block)[64];

    if (!image_finale) {
        fprintf(stderr, "Erreur : échec de l'allocation mémoire.\n");
        return EXIT_FAILURE;
    }

    // Décodage des MCUs
    if (nb_composantes < 2) {
        // Image en niveaux de gris (1 seule composante)
        for (int bloc_index = 0; bloc_index < nb_blocs; bloc_index++) {
            int bloc_y = bloc_index / blocs_par_ligne;
            int bloc_x = bloc_index % blocs_par_ligne;

            // Décodage et décompression du bloc 8x8
            decoder_bloc_8x8_composante(br, 0, mcu[0]);
            quantification_inverse(mcu[0], parsed.dqt_tables[0].values, iquant[0]);
            zigzag_inverse(iquant[0], matrix[0]);
            idct(matrix[0], bloc_image[0]);

            // Copier le bloc décodé dans l’image finale en respectant les dimensions
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    int x = bloc_x * 8 + j;
                    int y = bloc_y * 8 + i;
                    int idx_final = y * width + x;

                    if (x < width && y < height) {
                        image_finale[idx_final] = bloc_image[0][i * 8 + j];
                    }
                }
            }
        }
    } else {
        // Traitement des images couleur (Y, Cb, Cr
        int idx_Y = -1, idx_Cb = -1, idx_Cr = -1;

        // Identifier les indices des composantes Y, Cb et Cr
        for (int j = 0; j < 3; j++) {
            uint8_t id = parsed.sof0_header.component[j].identifiant;
            if (id == 1) idx_Y = j;
            else if (id == 2) idx_Cb = j;
            else if (id == 3) idx_Cr = j;
        }

        if (idx_Y == -1 || idx_Cb == -1 || idx_Cr == -1) {
            fprintf(stderr, "Erreur : IDs Y/Cb/Cr non trouvés dans les composantes.\n");
            return EXIT_FAILURE;
        }

        int indices[3] = {idx_Y, idx_Cb, idx_Cr};
        // Décodage, déquantification, dészigzag et IDCT pour chaque bloc MCU de chaque composante
        for (int bloc_index = 0; bloc_index < nb_blocs; bloc_index++) {
            int bloc_y = bloc_index / blocs_par_ligne;
            int bloc_x = bloc_index % blocs_par_ligne;
            int H_comp[3], V_comp[3];
            int bloc_idx = 0;

            for (int j = 0; j < 3; j++) {
                int idx = indices[j];
                H_comp[j] = parsed.sof0_header.component[idx].echant_horiz;
                V_comp[j] = parsed.sof0_header.component[idx].echant_vertic;
                int qt_idx = parsed.sof0_header.component[idx].quant_table_idx;

                for (int v = 0; v < V_comp[j]; v++) {
                    for (int h = 0; h < H_comp[j]; h++) {
                        decoder_bloc_8x8_composante(br, idx, mcu[bloc_idx]);
                        quantification_inverse(mcu[bloc_idx], parsed.dqt_tables[qt_idx].values, iquant[bloc_idx]);
                        zigzag_inverse(iquant[bloc_idx], matrix[bloc_idx]);
                        idct(matrix[bloc_idx], bloc_image[bloc_idx]);
                        bloc_idx++;
                    }
                }
            }

            // Calcul des indices de début dans bloc_image[] pour chaque composante
            int start_indices[3] = {
                0,
                H_comp[0] * V_comp[0],
                H_comp[0] * V_comp[0] + H_comp[1] * V_comp[1]
            };

            // Effectuer l'upsampling des blocs Y, Cb et Cr vers un format YCbCr intercalé
            upsample_mcu_blocks_ycbcr(upsampled_blocks, bloc_image, start_indices,
                                      H_comp, V_comp, Hmax, Vmax, total_blocs_mcu);

            if (!upsampled_blocks) {
                fprintf(stderr, "Erreur : échec de l'upsampling.\n");
                return EXIT_FAILURE;
            }

            // Conversion des blocs upsamplés YCbCr en blocs RGB pour chaque bloc i.
            for (int i = 0; i < H_V_max; i++) {
                for (int k = 0; k < 64; k++) {
                    bloc_ycbcr[i][0][k] = (uint8_t) upsampled_blocks[i][k];
                    bloc_ycbcr[i][1][k] = (uint8_t) upsampled_blocks[H_V_max + i][k];
                    bloc_ycbcr[i][2][k] = (uint8_t) upsampled_blocks[2 * H_V_max + i][k];
                }

                converted_block= conversion(bloc_ycbcr[i]);

                for (int c = 0; c < 3; c++)
                    memcpy(rgb[i][c], converted_block[c], 64);
            }

            // Placement dans image_finale (R, G, B)
            copier_blocs_rgb_dans_image(bloc_x,bloc_y, Hmax, Vmax, width, height, H_V_max, rgb, image_finale);
        }
    }

    // Écriture du fichier PPM
    FILE *fichier = fopen(output_filename, "wb");
    if (fichier) {
        ecriture(image_finale, fichier, width, height, nb_composantes);
        fclose(fichier);
    } else {
        fprintf(stderr, "Erreur : impossible de créer le fichier PPM\n");
    }

    // Libération mémoire
    free(image_finale);
    free(mcu);
    free(matrix);
    free(bloc_image);
    free(iquant);
    free(upsampled_blocks);
    for (int i = 0; i < 3; i++) {
        tab_huffman_free(huff_dc_tables[i]);
        tab_huffman_free(huff_ac_tables[i]);
    }
    free_bitstream(br);
    liberer_tables_huffman(&parsed);

    return EXIT_SUCCESS;
}
