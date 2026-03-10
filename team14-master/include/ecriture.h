#ifndef ECRITURE_H
#define ECRITURE_H
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void generation_nom_fichier( const char *filename,  char *output_filename, size_t output_size);
void copier_blocs_rgb_dans_image(int bloc_x, int bloc_y, int Hmax, int Vmax,
    int width, int height, int H_V_max,
    uint8_t rgb[H_V_max][3][64], int16_t *image_finale);
int ecriture(int16_t *bloc, FILE *fichier, int width, int height,int nb_composantes);

#endif 