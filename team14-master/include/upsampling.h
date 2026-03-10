#ifndef UPSAMPLING_H
#define UPSAMPLING_H

#include <stdint.h>
#include <stdlib.h>
#include "structures.h"

// Calcule les valeurs maximales de H (horizontal) et V (vertical) sampling
void max_down_sampling(int nb_components, Parsed_file *jpeg, uint8_t *max_h, uint8_t *max_v);
void nbr_blocs_mcu(Parsed_file *jpeg, int nb_components, int *nb_blocs_mcu);

                                   
void upsample_mcu_blocks_ycbcr(int8_t (*result)[64],
                                   uint8_t (*bloc_image)[64],
                                   int start_indices[3],
                                   int h_factors[3], int v_factors[3],
                                   int Hmax, int Vmax,
                                   int total_blocs_mcu) ;                                                                                                  
#endif // UPSAMPLING_H
