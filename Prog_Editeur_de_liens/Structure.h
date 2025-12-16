//Bibliothèque pour lecture de données ELF
#ifndef _STRUCTURE_H_
#define _STRUCTURE_H_

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <byteswap.h>
#include <string.h>


#define ENDIANESS_INDEX 5 // Index dans e_ident qui indique l'endianess
#define EI_DATA2LSB 1    // Valeur indiquant du little endian
#define EI_DATA2MSB 2    // Valeur indiquant du big endian

// *************************************************************************************************************
// ************************** Structures de données utilisées dans le programme **********************************
//liste des sections le poiteur la liste chainee il est e_shoff
typedef struct Shdr_liste Shdr_liste; 
struct Shdr_liste{
	Elf32_Shdr header;
	unsigned char * content;
	Shdr_liste * next;
};


typedef struct Symbol_list Symbol_list;
struct Symbol_list{
	Elf32_Sym * list;
	int nbSymbols;
};

// *************************************************************************************************************
// *********************** Fonctions de gestion de l'endianess du fichier ELF ***********************************
// Retourne 1 si l'endianess du header est en big endian, 0 dans le cas contraire
int is_big_endian(Elf32_Ehdr h);



// Lis un header de section 
int read_Elf32_Shdr( FILE *f, Elf32_Ehdr h, unsigned int index, Elf32_Shdr * s);
// Lis une structure Elf32_Sym
int read_Elf32_Sym( FILE *f, Elf32_Ehdr h, Elf32_Sym *s );


//****************************************************************************************************************** 
//*******************************************AFFICHAGE entete*******************************************************
int read_Elf32_Ehdr( FILE *f, Elf32_Ehdr * h );
void afficher_Elf32_Ehdr( Elf32_Ehdr h );
void afficher_type(Elf32_Ehdr h);
void afficher_machine(Elf32_Ehdr h);
void afficher_version(Elf32_Ehdr h);
void afficher_indent(Elf32_Ehdr h);
void afficher_en_tete_format_commande(Elf32_Ehdr h);

//******************************************************************************************************************
//********************************************Manipulation des sections ****************************************** */
int read_Elf32_Shdr(FILE *f, Elf32_Ehdr h, unsigned int index, Elf32_Shdr *s);
char *read_shstrtab(FILE *f, Elf32_Ehdr h);
void read_Shdr_list(FILE *f, Elf32_Ehdr h, Shdr_liste * L);
void afficher_Shdr_type(Elf32_Shdr s);
void afficher_section_flags(Elf32_Word flags);
void afficher_Shdr_list(FILE *f, Elf32_Ehdr h, Shdr_liste * L);

/******************************************************************************************************************* */
/*************************************************** */


#endif _STRUCTURE_H_