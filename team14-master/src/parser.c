#include "parser.h"
#include "structures.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* Sauter N octets */
void skip_bytes(FILE *f, int n) {
    for (int i = 0; i < n; i++) fgetc(f);
}
void lire_entete(FILE *jpeg_file, Parsed_file *parsed){
    /*lecture et verficiation de de SOI*/
    /*boucle sur les marquers jusq'a SOS*/
    uint8_t byte;
    fread(&byte, sizeof(byte), 1, jpeg_file);
    if (byte !=0xFF ){
        fprintf(stderr,"erreur : fichier ne commence pas par SOI");
        exit(1);
    }
    fread(&byte, sizeof(byte), 1, jpeg_file);
    if (byte != 0xD8) {
        fprintf(stderr, "Erreur: marqueur SOI incorrect.\n");
        exit(1);
    }
    lire_SOI(jpeg_file,parsed);
    while (1){
        int byte1=fgetc(jpeg_file);
        /*verification de deuxieme marqueur */
        if (byte1==EOF) break;
        int byte2;
        do {
            /*ignorer les Oxff répétés */
            byte2 =fgetc(jpeg_file);
            if (byte2 == EOF) return;
        }while (byte2==0xFF);
        // Vérification du marqueur EOI (0xFFD9)
        // Vérification du marqueur EOI (0xFFD9)
        if (byte2 == 0xD9) {
            lire_EOI(jpeg_file);  // Appel à la fonction dédiée à EOI
            return;
        }
        unsigned short marker = 0xFF00 | byte2;
        printf("[Marker] Processing marker 0x%X\n", marker);
        switch (marker){
            case 0XFFE0:
                lire_app0(jpeg_file, &parsed->app0_section);
                break;
            case 0XFFDB:
                lire_DQT(jpeg_file,parsed);
                break;
            case 0XFFC0:
                lire_SOF0(jpeg_file,parsed);
                break;
            case 0XFFC4:
                lire_DHT(jpeg_file,parsed);
                break;
            case 0XFFDA:
                lire_SOS(jpeg_file,parsed);
                return ;
            case 0xFFFE: // COM (commentaire)
                lire_COM(jpeg_file);
                break;
            case 0xFFE1 ... 0xFFEF: // APP1 à APP15
            case 0xFFDD: // DRI
            default: {
                // Sections à ignorer proprement
                uint8_t len_hi = fgetc(jpeg_file);
                uint8_t len_lo = fgetc(jpeg_file);
                unsigned short len = (len_hi << 8) | len_lo;
                skip_bytes(jpeg_file, len - 2);
                break;
            }
        }
    }
    // Si on est ici, c'est que la lecture est terminée
    printf("[EOI] marker found\n");
    printf("bitstream empty\n");
}
void lire_SOI(FILE *file, Parsed_file *parsed){
    /*rien car SOI est deja lu */
    (void)file;   // Signale que 'file' n'est pas utilisé
    (void)parsed; // Signale que 'parsed' n'est pas utilisé
    printf("[SOI] marker found\n");
}
void lire_app0(FILE *jpeg_file, APP0_section *app0) {
    uint8_t  length[2], jfif_header[5], version[2];
    // Lire la taille de la section (2 octets)
    if (fread(length, 1, 2, jpeg_file) != 2) {
        fprintf(stderr, "Erreur : Lecture de la longueur APP0 échouée\n");
        exit(EXIT_FAILURE);
    }
    app0->length = (length[0] << 8) | length[1];  // Ajout de la longueur dans la structure
    // Trace de la section APP0
    printf("[APP0] length %d bytes\n", app0->length);
    printf("\tJFIF application\n");

    // Lire "JFIF\0"
    if (fread(jfif_header, 1, 5, jpeg_file) != 5 || strncmp((char *)jfif_header, "JFIF\0", 5) != 0) {
        fprintf(stderr, "Erreur : En-tête JFIF incorrect\n");
        exit(EXIT_FAILURE);
    }

    // Lire la version JFIF (X.Y)
    if (fread(version, 1, 2, jpeg_file) != 2) {
        fprintf(stderr, "Erreur : Lecture de la version JFIF échouée\n");
        exit(EXIT_FAILURE);
    }

    app0->version_major = version[0];
    app0->version_minor = version[1];
    printf("\tVersion: %d.%d\n", app0->version_major, app0->version_minor);
    // Vérifier que la version est 1.1
    if (app0->version_major != 1 || app0->version_minor != 1) {
        fprintf(stderr, "Erreur : Version JFIF incorrecte (attendu 1.1)\n");
        exit(EXIT_FAILURE);
    }

    // Ignorer les 7 octets restants (données spécifiques JFIF)
    fseek(jpeg_file, 7, SEEK_CUR);

    // Copier "JFIF" dans la structure
    memcpy(app0->jfif_marker, jfif_header, 5);
    app0->jfif_marker[5] = '\0';
}
uint16_t custom_ntohs(uint16_t value) {
    return (value >> 8) | (value << 8);
}
void lire_COM(FILE *file) {
    uint16_t comment_length;
    fread(&comment_length, sizeof(comment_length), 1, file);
    comment_length = custom_ntohs(comment_length);  // Convertir en ordre d'octets hôte

    size_t real_comment_length = comment_length - 2;
    char* comment = malloc(real_comment_length + 1); // +1 pour le caractère nul
    if (comment == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour le commentaire.\n");
        exit(1);
    }

    if (fread(comment, sizeof(char), real_comment_length, file) != real_comment_length) {
        fprintf(stderr, "Erreur : Lecture incomplète du commentaire.\n");
        free(comment);
        exit(1);
    }
    comment[real_comment_length] = '\0';  // Terminaison propre de la chaîne

    // Affichage formaté
    printf("[COM] length %d bytes\n", comment_length);
    printf("\tComment found: \"%s\"\n", comment);

    free(comment);
}

void lire_DQT(FILE *file, Parsed_file *parsed){
    uint16_t length =(fgetc(file)<<8) |fgetc(file);
    length-=2;
    while (length >0){
        uint8_t precindex=fgetc(file);
        uint8_t precision=precindex >>4;// les 4 bits de gauches
        uint8_t index = precindex & 0x0F;// les 4 bits de droite
        parsed->dqt_tables[index].precision = precision;
        parsed->dqt_tables[index].indice_iq = index;

        int nb_octets = (precision == 0) ? 64 : 128;
        // Affichage du format souhaité
        printf("[DQT] length %d bytes\n", length + 2); // Ajout des 2 octets pour la longueur
        printf("\tquantization table index %d\n", index);
        printf("\tquantization precision %d bits\n", precision == 0 ? 8 : 16);
        printf("\tquantization table read (%d bytes)\n", nb_octets);

        //On lit les 64 coefficients de la table et on les range dans le tableau
        for (int i = 0; i < 64; i++) {
            if (precision == 0) {
                parsed->dqt_tables[index].values[i] = fgetc(file);
            } else {
                uint16_t val = (fgetc(file) << 8) | fgetc(file); // 16 bits
                parsed->dqt_tables[index].values[i] = val; 
            }      
        }

        length -= 1 + nb_octets; // On enlève ce qu'on a lu
    }

}
const char* get_component_name(uint8_t id){
    switch (id){
        case 1: return "Y";
        case 2: return "Cb";
        case 3 :return "Cr";
        default :return "Unknown";
    }
}
void lire_SOF0(FILE *file, Parsed_file *parsed){
    uint16_t length =(fgetc(file)<<8) |fgetc(file);
    parsed->sof0_header.length=length;
    parsed->sof0_header.precision = fgetc(file);
    parsed->sof0_header.hauteur =(fgetc(file)<<8) |fgetc(file);
    parsed->sof0_header.largeur =(fgetc(file)<<8) |fgetc(file);
    parsed->sof0_header.nbr_composantes=fgetc(file);
    parsed->image_hauteur = parsed->sof0_header.hauteur;
    parsed->image_largeur = parsed->sof0_header.largeur;
    // Affichage du format attendu
    printf("[SOF0] length %d bytes\n", length);
    printf("\tsample precision %d\n", parsed->sof0_header.precision);
    printf("\timage height %d\n", parsed->sof0_header.hauteur);
    printf("\timage width %d\n", parsed->sof0_header.largeur);
    printf("\tnb of component %d\n", parsed->sof0_header.nbr_composantes);
    for (int i =0;i < parsed->sof0_header.nbr_composantes;i++){
        Component *comp = &parsed->sof0_header.component[i];
        comp->identifiant = fgetc(file);
        uint8_t echant = fgetc(file);
        comp->echant_horiz = echant >> 4;
        comp->echant_vertic = echant & 0x0F;
        comp->quant_table_idx = fgetc(file);
        // Affichage des informations du composant
        printf("component %s\n", get_component_name(comp->identifiant)); 
        printf("\tid %d\n", comp->identifiant);
        printf("\tsampling factors (hxv) %dx%d\n", comp->echant_horiz, comp->echant_vertic);
        printf("\tquantization table index %d\n", comp->quant_table_idx);
    }
}
void lire_DHT(FILE *file, Parsed_file *parsed){
    uint16_t length =(fgetc(file)<<8) |fgetc(file);
    uint16_t original_length = length; // Conservez la longueur d'origine
    length-=2;
    while (length>0) {
        uint8_t info=fgetc(file);
        uint8_t info_non_util=info &  0XE0;// les 3 bits de gauches
        if (info_non_util !=0x00 ){
            fprintf(stderr,"erreur : la section  DHT ne contient pas les trois bits non utilisées sous le bon format ");
            exit(EXIT_FAILURE);
        }
        uint8_t type =(info & 0X10) >>4;// bit 4
        uint8_t indice=info & 0X0F;// bit 3-0
        Huffman_table* table_huff =&parsed->huffman_tables[indice][type];
        table_huff->type=type; 
        table_huff->indice=indice;
        // Trace de la table DHT
        printf("[DHT] length %d bytes\n", original_length ); // +2 pour les 2 octets de longueur
        printf("\tHuffman table type %s\n", type == 0 ? "DC" : "AC");
        printf("\tHuffman table index %d\n", indice);
        int total_symbols = 0;
        for (int i = 0; i < 16; ++i){
            table_huff->lengths[i]=fgetc(file);
            total_symbols += table_huff->lengths[i];
        }
        // Trace des symboles et des longueurs
        printf("\tTotal nb of Huffman symbols %d\n", total_symbols);
        table_huff->symbols = malloc(total_symbols * sizeof(unsigned char));
        for (int i = 0; i < total_symbols; ++i) {
            table_huff->symbols[i] = fgetc(file);
        }
        length-=1+16+total_symbols;
        original_length = length + 2;  // Réajuster la longueur d'origine pour chaque segment
    }   
}
void lire_SOS(FILE *file, Parsed_file *parsed){
    uint16_t length =(fgetc(file)<<8) |fgetc(file);
    parsed->sos_header.length=length;
    // Trace de la longueur du segment SOS
    printf("[SOS] length %d bytes\n", length);

    parsed->sos_header.nbr_composantes=fgetc(file);
    printf("\tnb of components in scan %d\n", parsed->sos_header.nbr_composantes);
    for (int i =0;i < parsed->sos_header.nbr_composantes;i++){
        uint8_t id =fgetc(file);
        uint8_t huff=fgetc(file);//les deux indices de HUffman
        parsed->sos_header.composantes[i].id=id;
        parsed->sos_header.composantes[i].huffman_AC=huff>>4;//les 4 bits de gauche 
        parsed->sos_header.composantes[i].huffman_DC=huff & 0X0F;//les 4 bits de droite
        // Affichage des informations sur le composant
        printf("\tscan component index %d\n", i);
        printf("\t\tassociated to component of id %d (frame index %d)\n", id, i);
        printf("\t\tassociated to DC Huffman table of index %d\n", parsed->sos_header.composantes[i].huffman_DC);
        printf("\t\tassociated to AC Huffman table of index %d\n", parsed->sos_header.composantes[i].huffman_AC);
    }
    parsed->sos_header.Ss=fgetc(file);
    parsed->sos_header.Se=fgetc(file);
    uint8_t ahal=fgetc(file);
    parsed->sos_header.Ah=ahal>>4;
    parsed->sos_header.Al=ahal & 0X0F;
    // Affichage des paramètres ignorés
    printf("\tother parameters ignored (3 bytes)\n");

    // Fin de l'en-tête SOS
    printf("\tEnd of Scan Header (SOS)\n");

}
void lire_EOI(FILE *jpeg_file) {
    uint8_t byte;
    // Vérifier le marqueur EOI 0xFFD9
    fread(&byte, sizeof(byte), 1, jpeg_file);
    if (byte != 0xFF) {
        fprintf(stderr, "Erreur : Marqueur EOI mal formaté.\n");
        exit(1);
    }
    fread(&byte, sizeof(byte), 1, jpeg_file);
    if (byte != 0xD9) {
        fprintf(stderr, "Erreur : Marqueur EOI incorrect.\n");
        exit(1);
    }
    printf("[EOI] marker found\n");
    printf("bitstream empty\n");
}
void lire_flux(FILE *jpeg_file, Parsed_file* parsed) {
    uint8_t byte;
    size_t buffer_size = 0;
    size_t capacity = 1024;
    uint8_t *buffer = malloc(capacity);
    
    if (!buffer) {
        fprintf(stderr, "Erreur d'allocation mémoire.\n");
        exit(EXIT_FAILURE);
    }

    printf("[Flux] Début de la lecture du flux après SOS...\n");

    while (fread(&byte, sizeof(byte), 1, jpeg_file) == 1) {
        if (buffer_size >= capacity) {
            capacity *= 2;
            buffer = realloc(buffer, capacity);
            if (!buffer) {
                fprintf(stderr, "Erreur de réallocation mémoire.\n");
                exit(EXIT_FAILURE);
            }
        }

        buffer[buffer_size++] = byte;

        // Vérifier EOI en lisant mais sans afficher EOI immédiatement
        if (byte == 0xFF) {
            uint8_t next_byte;
            fread(&next_byte, sizeof(next_byte), 1, jpeg_file);
            if (next_byte == 0xD9) {  // Marqueur EOI détecté
                // Arrêter la lecture ici mais NE PAS afficher EOI maintenant
                break;
            }
        }
    }

    // Stocker les données du flux dans `parsed`
    parsed->flux = buffer;
    parsed->flux_length = buffer_size;

    //  **Affichage du flux avant EOI**
    printf("[Flux] Contenu du flux après SOS (%zu octets) :\n", buffer_size);
    for (size_t i = 0; i < buffer_size; i++) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");

    // Maintenant seulement, afficher EOI
    printf("[EOI] Marqueur EOI détecté. Fin du flux.\n");
}
void liberer_tables_huffman(Parsed_file *parsed) {
    for (int indice = 0; indice < 4; indice++) {
        for (int type = 0; type < 2; type++) {
            Huffman_table *table = &parsed->huffman_tables[indice][type];
            if (table->symbols != NULL) {
                free(table->symbols);
                table->symbols = NULL;
            }
        }
    }
}
