#ifndef IDCT_H
#define IDCT_H

#include <stdint.h>
#include <stddef.h>


// Applique l'IDCT sur un bloc de 64 coefficients


void idct(int16_t matrix[8][8], uint8_t bloc_image[64]);
#endif

