#include "quantification.h"
#include <stdint.h>

//Quantification inverse d'un bloc dct quantifié (bloc)
void quantification_inverse(int16_t bloc[64], const uint8_t qtable[64] , int16_t iquant[64]) {

    //Parcours des 64 coefficients dans bloc
    for (int i = 0; i < 64; i++) {
        //Multiplication de chaque coeff par son facteur dans la table de quantification "iquant"
        iquant[i]=bloc[i] * qtable[i];
    }
}