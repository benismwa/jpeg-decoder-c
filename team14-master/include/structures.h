#ifndef STRUCTURES_H
#define STRUCTURES_H
#include <stdint.h>
#include <stddef.h> // Définit size_t
/*structure pour une section jpeg quelconque*/
typedef struct {
    uint16_t maker;
    uint16_t length;
    uint8_t* data;

}JPEG_section;
typedef struct {
    uint16_t length;
    char jfif_marker[5];  // 'J', 'F', 'I', 'F', '\0'
    uint8_t version_major;  // Version majeure de JFIF (devrait être 1)
    uint8_t version_minor;  // Version mineure de JFIF (devrait être 1)
} APP0_section;

/*structure qui sert a definir les tables de quantification DQT qui seront utilisés dasns la compression */

typedef struct {
    uint16_t length;
    uint8_t precision; /*Précision (0 : 8 bits, 1 : 16 bits)*/
    uint8_t indice_iq;
    uint8_t values[64];
}Quantification_table;

/*structure de Huffman table */
typedef struct {
    uint16_t length;
    uint8_t type; // DC ou AC
    uint8_t indice; // indice de la table (0..3)
    uint8_t lengths[16];    // nombre de symboles de chaque longueur
    unsigned char *symbols ;// tableau dynamique des symboles
}Huffman_table;
typedef struct{
    uint8_t identifiant;
    uint8_t echant_horiz;
    uint8_t echant_vertic;
    uint8_t quant_table_idx;
}Component;
/*structure de SOFx */
typedef struct {
    uint16_t length;
    uint8_t precision;
    uint16_t hauteur;
    uint16_t largeur ;
    uint8_t nbr_composantes;/*Nombre de composantes N (Ex : 3 pour le YCbCr, 1 pour les niveaux de gris)*/
    Component component[3];
}SOF_tete;
/*structure de SOS */
typedef struct
{
    uint8_t id;
    uint8_t huffman_DC;
    uint8_t huffman_AC;
}SOS_Component;

typedef struct{
    uint16_t length;
    uint8_t nbr_composantes;
    SOS_Component composantes[4];
    uint8_t Ss;
    uint8_t Se;
    uint8_t Ah;
    uint8_t Al;

}SOS_tete;
typedef int16_t Block[64];
typedef struct {
    APP0_section app0_section;
    Quantification_table dqt_tables[4];
    SOF_tete sof0_header;
    Huffman_table huffman_tables[4][2]; //4 car 4 tables possibles (Y,Cb,Cr)et 2 car deux types de tables de Huffman(DC et AC)
    SOS_tete sos_header;
    uint8_t *flux;  //champ pour le flux JPEG
    size_t flux_length;  
    uint16_t image_largeur;//largeur de l'image
    uint16_t image_hauteur;//hauteur de l'image 
} Parsed_file;
#endif