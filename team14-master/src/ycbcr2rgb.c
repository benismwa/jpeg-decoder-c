#include "ycbcr2rgb.h"
#include <stdint.h>
#include <math.h>

//pour des valeurs entre 0 et 255
static uint8_t clamp(int val) {
 if (val < 0) return 0;
 if (val > 255) return 255;
 return (uint8_t)val;
}
// retourne un tableau static uint8_t blocs[3][64] avec :blocs[0] = R des pixels ,blocs[1] = G des pixels ,blocs[2] = B des pixels.
uint8_t (*conversion(uint8_t blocs_ycbcr[3][64]))[64]{
 static uint8_t blocs[3][64];
    for (int i=0 ;i < 64;i++){
    uint8_t y = blocs_ycbcr[0][i];
    uint8_t cb =blocs_ycbcr[1][i];
    uint8_t cr =blocs_ycbcr[2][i];
   int r = y + 1.402 * (cr - 128);
   int g = y - 0.34414 * (cb - 128) - 0.71414 * (cr - 128);
   int b = y + 1.772 * (cb - 128);
    // pour chaque pixel on stocke 3 composantes R G et B
    blocs[0][i]=(uint8_t)(clamp(r));
    blocs[1][i]=(uint8_t)(clamp(g));
    blocs[2][i]=(uint8_t)(clamp(b));

    }
    return blocs;// Pointeur vers un tableau [3][64]
    }