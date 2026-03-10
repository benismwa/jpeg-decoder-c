#ifndef PARSER_H
#define PARSER_H
#include "structures.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h> // Définit size_t
void lire_entete(FILE *jpeg_file,Parsed_file* parsed);
void lire_app0(FILE *jpeg_file,APP0_section *app0);
void lire_COM(FILE *jpeg_file);
void lire_SOI(FILE *jpeg_file,Parsed_file* parsed);
void lire_DQT(FILE *jpeg_file,Parsed_file* parsed);
void lire_DHT(FILE *jpeg_file,Parsed_file* parsed);
void lire_SOF0(FILE *jpeg_file,Parsed_file* parsed);
void lire_SOS(FILE *jpeg_file,Parsed_file* parsed);
void lire_EOI(FILE *jpeg_file);
void lire_flux(FILE *jpeg_file, Parsed_file* parsed);
void liberer_tables_huffman(Parsed_file *parsed);
#endif