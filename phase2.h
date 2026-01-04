//Bibliothèque pour la phase 2 du linker ELF
#ifndef _PHASE2_H_
#define _PHASE2_H_

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <byteswap.h>
#include <string.h>

/*************************************ETAPE6************************************************************** */
/******************************************************************************************************** */
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

/*************************************ETAPE7************************************************************** */
/******************************************************************************************************** */
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



/*************************************ETAPE8************************************************************** */
/******************************************************************************************************** */

typedef struct {
  char *name;              /* nom de section relocation: ".rel.text" */
  int target_out_index;    /* index section cible dans OUT (renuméroté) */

  Elf32_Rel *rels;
  size_t n;
  size_t cap;
} RelAgg;


int E8_fusionner_corriger_relocations(const char *fileA,
                                      const char *fileB,
                                      const char *fileOut7,
                                      const char *fileOut8,
                                      const uint32_t *renumA, size_t lenA,
                                      const uint32_t *renumB, const uint32_t *deltaB, size_t lenB,
                                      const uint32_t *renumSymA, size_t nSymA,
                                      const uint32_t *renumSymB, size_t nSymB);

/*************************************ETAPE9************************************************************** */
/******************************************************************************************************** */

int E9_produire_elf_final(const char *fileOut8, const char *fileOut9);

#endif /*_PHASE2_H_*/