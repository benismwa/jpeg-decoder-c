#include "ecriture.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Génère le nom du fichier de sortie en remplaçant l'extension par ".ppm"
void generation_nom_fichier(const char *filename, char *output_filename, size_t output_size) {
    const char *dot = strrchr(filename, '.');
    if (dot) {
        size_t len = dot - filename;
        if (len >= output_size) len = output_size - 1;
        strncpy(output_filename, filename, len);
        output_filename[len] = '\0';
        strncat(output_filename, ".ppm", output_size - len - 1);
    } else {
        snprintf(output_filename, output_size, "%s.ppm", filename);
    }
}

// Copie les blocs RGB décodés dans le buffer image finale
void copier_blocs_rgb_dans_image(int bloc_x, int bloc_y, int Hmax, int Vmax,
    int width, int height, int H_V_max,
    uint8_t rgb[H_V_max][3][64], int16_t *image_finale) {

    for (int i = 0; i < H_V_max; i++) {
        int local_bloc_y = i / Hmax;
        int local_bloc_x = i % Hmax;

        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                int x = (bloc_x * Hmax + local_bloc_x) * 8 + c;
                int y = (bloc_y * Vmax + local_bloc_y) * 8 + r;

                if (x < width && y < height) {
                    int idx_final = (y * width + x) * 3;
                    int pixel_idx = r * 8 + c;

                    image_finale[idx_final + 0] = rgb[i][0][pixel_idx];
                    image_finale[idx_final + 1] = rgb[i][1][pixel_idx];
                    image_finale[idx_final + 2] = rgb[i][2][pixel_idx];
                }
            }
        }
    }
}

// Écrit l'image dans un fichier PGM (niveau de gris) ou PPM (couleur)
int ecriture(int16_t *bloc, FILE *fichier, int width, int height, int nb_composantes) {
    if (nb_composantes < 2) {
        // Image en niveaux de gris (PGM)
        fprintf(fichier, "P5\n%d %d\n255\n", width, height);

        int nb_pixels = width * height;
        for (int i = 0; i < nb_pixels; i++) {
            int val = bloc[i];
            if (val < 0) val = 0;
            if (val > 255) val = 255;

            uint8_t val8 = (uint8_t)val;
            fwrite(&val8, sizeof(uint8_t), 1, fichier);
        }
    } else {
        // Image couleur (PPM)
        fprintf(fichier, "P6\n%d %d\n255\n", width, height);

        int nb_octets = width * height * 3;
        for (int i = 0; i < nb_octets; i++) {
            int val = bloc[i];
            if (val < 0) val = 0;
            if (val > 255) val = 255;

            uint8_t val8 = (uint8_t)val;
            fwrite(&val8, sizeof(uint8_t), 1, fichier);
        }
    }
    return 0;
}
