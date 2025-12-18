//Bibliothèque pour lecture de données ELF
#ifndef _PHASE1_H_
#define _PHASE1_H_

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
int is_big_endian_fich(Elf32_Ehdr h);




//****************************************************************************************************************** 
//*******************************************ETAPE 1 :AFFICHAGE entete*******************************************************
int E1_read_Elf32_Ehdr(FILE *f, Elf32_Ehdr *h);
const char *machine_to_string(Elf32_Ehdr h);
const char *type_to_string(Elf32_Ehdr h);
int E1_afficher_read_Elf32_Ehdr(Elf32_Ehdr header);
//******************************************************************************************************************
//********************************************ETAPE 2: Table des sections ****************************************** */
int E2_read_Elf32_Shdr(FILE *f, Elf32_Ehdr h, unsigned int index, Elf32_Shdr *s);
char *read_shstrtab(FILE *f, Elf32_Ehdr h);
void E2_read_Shdr_list(FILE *f, Elf32_Ehdr h, Shdr_liste * L);
void afficher_Shdr_type(Elf32_Shdr s);
void afficher_section_flags(Elf32_Word flags);
void E2_afficher_Shdr_list(FILE *f, Elf32_Ehdr h, Shdr_liste *L);
//******************************************************************************************************************
//********************************************ETAPE 3: Contenu des sections ****************************************** */
Shdr_liste* section_index(Shdr_liste *L, int idx) ;
Shdr_liste* section_name(FILE *f, Elf32_Ehdr h, Shdr_liste *L, const char *target);
void E3_afficher_content_section(Shdr_liste *section) ;

/******************************************************************************************************************* */
/*************************************************** ETAPE4 : Table des symboles****************************************** */
const char *sym_bind_to_string(unsigned char info);
const char *sym_type_to_string(unsigned char info);
const char *sym_visibility_to_string(unsigned char info);
void correct_endian_sym(Elf32_Sym *s);
void E4_afficher_symtab(FILE *f, Elf32_Ehdr h, Shdr_liste *L);
/***************************************************************************************************************** */
/*************************************************** ETAPE5: Table des relocations******************************************** */
const char *arm_rel_type(unsigned type);
void E5_afficher_relocation(FILE *f, Elf32_Ehdr h, Shdr_liste *L);
/***************************************************************************************************************** */
void free_Shdr_list(Shdr_liste *L);
#endif /*_PHASE1_H_*/
