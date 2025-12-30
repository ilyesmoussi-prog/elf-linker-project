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






 /* SecR = une section dans le fichier résultat :
 *  - name : nom de section (sera mis dans .shstrtab)
 *  - sh   : header Elf32_Shdr (en endian hôte)
 *  - data : contenu binaire si ce n’est pas NOBITS, sinon NULL
 */
typedef struct {
  char *name;
  Elf32_Shdr sh;
  unsigned char *data;
  uint32_t data_size;
} SecR;

/*
  Un petit “vector” dynamique (table extensible) de sections résultat.
 */
typedef struct {
  SecR *v;
  int n, cap;
} VecSec;

uint32_t align_up(uint32_t x, uint32_t a);
char *dupliquer_chaine(const char *s);
int contient_debug(const char *name);
int est_comment(const char *name);
int est_shstrtab(const char *name);
const char *get_nom_section(const char *shstrtab, Elf32_Shdr sh);
int section_a_ignorer(const char *name, const Elf32_Shdr *sh);
void convertir_ehdr_pour_sortie(Elf32_Ehdr *h, int big_endian_out);
void convertir_shdr_pour_sortie(Elf32_Shdr *s, int big_endian_out);
void vec_init(VecSec *a);
void vec_free(VecSec *a);
int vec_push(VecSec *a, SecR s);
int vec_find_by_name(const VecSec *a, const char *name);
unsigned char *construire_shstrtab(VecSec *R, uint32_t *out_size);
int charger_elf(const char *path,FILE **f_out,Elf32_Ehdr *eh_out,Shdr_liste **L_out,char **shstr_out);
int ajouter_section0(VecSec *R);
int ajouter_sections_de_A(VecSec *R,Elf32_Ehdr ehA, Shdr_liste *LA,const char *shstrA,uint32_t *renumA);
int traiter_sections_de_B(VecSec *R,Elf32_Ehdr ehB, Shdr_liste *LB,const char *shstrB,uint32_t *renumB,uint32_t *deltaB);
int ajouter_shstrtab(VecSec *R, int *out_shstrndx);
void correction_minimale_sh_link(VecSec *R);
int ecrire_elf_resultat(const char *fileOut,const Elf32_Ehdr *eh_base,VecSec *R,int shstrndx,int out_big);
int E6_fusionner_sections(const char *fileA, const char *fileB, const char *fileOut,uint32_t **renumA_out, size_t *lenA_out,uint32_t **renumB_out, uint32_t **deltaB_out, size_t *lenB_out);

/******************************************************************** */
/***********************************Table de symboles************************ */

typedef struct {
  Elf32_Sym  *syms;//table des symboles
  size_t      nsyms;//nombre de symboles
  char       *strtab;//table des chaines
  size_t      strsz;//taille de la table des chaines
} TableSym;
/**structure strtab*** */
typedef struct {
  unsigned char *data;
  size_t         sz;
  size_t         cap;
} Buf;
/* Structure pour stocker les symboles dans out */
typedef struct {
  Elf32_Sym *v;
  size_t     n;
  size_t     cap;
} VecSym;


int sym_est_local(const Elf32_Sym *s);
int sym_est_undef(const Elf32_Sym *s);
int sym_est_special(const Elf32_Sym *s);
int sym_est_defini_en_section(const Elf32_Sym *s);
int sym_pointe_section_ignorée(const Elf32_Sym *s, const uint32_t *renum, size_t renum_len);
void free_table_sym(TableSym *t);
Shdr_liste* trouver_section_par_nom(Shdr_liste *L, const char *shstr, const char *nom);
int charger_table_sym(FILE *f, Elf32_Ehdr eh, Shdr_liste *L, char *shstr, TableSym *out) ;
int buf_init(Buf *b);
void buf_free(Buf *b);
int buf_reserve(Buf *b, size_t need) ;
uint32_t strtab_ajouter(Buf *b, const char *s) ;
int vecsym_init(VecSym *vs) ;
void vecsym_free(VecSym *vs);
int vecsym_push(VecSym *vs, const Elf32_Sym *s);
int trouver_global_par_nom(const VecSym *out, const char *out_strtab, const char *name);
int corriger_symbole_pour_out(Elf32_Sym *dst,const Elf32_Sym *src,const uint32_t *renumSec, size_t renumSec_len,const uint32_t *deltaSec ,const char *src_strtab,Buf *out_strtab);
int construire_vecsec_depuis_liste(VecSec *R, Shdr_liste *L, const char *shstrtab);
int E7_fusionner_corriger_symboles(const char *fileA,
                                  const char *fileB,
                                  const char *fileOut6,
                                  const char *fileOut7,
                                  const uint32_t *renumA, size_t lenA,
                                  const uint32_t *renumB, const uint32_t *deltaB, size_t lenB,
                                  uint32_t **renumSymA_out, size_t *nSymA_out,
                                  uint32_t **renumSymB_out, size_t *nSymB_out);







#endif /*_PHASE1_H_*/
