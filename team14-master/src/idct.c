#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "idct.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif
#ifndef SQRT2
#define SQRT2 1.4142135623730951
#endif

double I_0(double a, double b) {
    return (a + b)/2.0;
}
double I_1(double a, double b) {
    return (a - b)/2.0;
}
void butterfly(double* y, double* z, double a, double b, double k, int n) {
    *y = (a * cos(n * PI / 16) - b * sin(n * PI / 16)) / k;
    *z = (b * cos(n * PI / 16) + a * sin(n * PI / 16)) / k;
}

void idct_1d(double input[8], double result[8]) {
    double a[8];
    for (int i = 0; i < 8; i++) {
        a[i] = input[i];
    }
    double b[8];

    //4 à 3
    b[0] = a[0];
    b[1] = a[4];
    b[2] = a[2];
    b[3] = a[6];
    b[4] = I_1(a[1], a[7]);
    b[5] = a[3] / SQRT2;
    b[6] = a[5] / SQRT2;
    b[7] = I_0(a[1], a[7]);

    // 3 à 2
    a[0] = I_0(b[0], b[1]);
    a[1] = I_1(b[0], b[1]);
    butterfly(&a[2], &a[3], b[2], b[3], SQRT2, 6);
    a[4] = I_0(b[4], b[6]);
    a[5] = I_1(b[7], b[5]);
    a[6] = I_1(b[4], b[6]);
    a[7] = I_0(b[7], b[5]);

    // 2 à 1
    b[0] = I_0(a[0], a[3]);
    b[1] = I_0(a[1], a[2]);
    b[2] = I_1(a[1], a[2]);
    b[3] = I_1(a[0], a[3]);
    butterfly(&b[4], &b[7], a[4], a[7], 1, 3);
    butterfly(&b[5], &b[6], a[5], a[6], 1, 1);

    // 1 à 0
    result[0] = I_0(b[0], b[7]);
    result[1] = I_0(b[1], b[6]);
    result[2] = I_0(b[2], b[5]);
    result[3] = I_0(b[3], b[4]);
    result[4] = I_1(b[3], b[4]);
    result[5] = I_1(b[2], b[5]);
    result[6] = I_1(b[1], b[6]);
    result[7] = I_1(b[0], b[7]);
}

void idct(int16_t matrix[8][8], uint8_t bloc_image[64]) {
    double temp[8][8];
    double resultat[8][8];
//Sur les lignes
    for (int x = 0; x < 8; x++) {
        double ligne[8];
        for (int i = 0; i < 8; i++) {
            ligne[i] = (double)(matrix[x][i]);
        }
        idct_1d(ligne, temp[x]);
    }
//Sur les colonnes
    for (int x = 0; x < 8; x++) {
        double colonne[8];
        for (int i = 0; i < 8; i++) {
            colonne[i] = (temp[i][x]);
        }
        double res[8];
        idct_1d(colonne, res);
        for (int y = 0; y < 8; y++) {
            resultat[y][x] = res[y];
        }
    }

 //Normalisation et clamping
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            double val = (resultat[i][j] * 8 + 128);
            if (val < 0) val = 0;
            if (val > 255) val = 255;
            bloc_image[i*8+j] = (uint8_t)(round(val));
        }
    }
}
