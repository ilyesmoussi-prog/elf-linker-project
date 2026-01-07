#include "phase1.h"
#include "util.h"
#include "phase2.h"


//aligner la section dans le fich resultat selon le champ d'alignement sh_addralign
uint32_t align_up(uint32_t x, uint32_t a)
{
  if (a == 0) a = 1;
  uint32_t r = x % a;
  return (r == 0) ? x : (x + (a - r));
}

// * Utile ici pour stocker durablement les noms de sections (.text, .data, ...) dans la
 /* structure résultat, car les pointeurs vers shstrtab (A/B) deviennent invalides après free(). */
char *dupliquer_chaine(const char *s)
{
  if (!s) s = "";
  size_t n = strlen(s) + 1;
  char *p = (char*)malloc(n);
  if (!p) return NULL;
  memcpy(p, s, n);
  return p;
}

/* Renvoie 1 si le nom contient ".debug", 0 sinon. */
 int contient_debug(const char *name)
{
  return (name && strstr(name, ".debug") != NULL);
}

/* Renvoie 1 si c’est la section ".comment". */
 int est_comment(const char *name)
{
  return (name && strcmp(name, ".comment") == 0);
}

/* Renvoie 1 si c’est la section ".shstrtab" (qu’on reconstruit). */
int est_shstrtab(const char *name)
{
  return (name && strcmp(name, ".shstrtab") == 0);
}

/* Récupère le nom de section à partir de shstrtab + sh_name. */
const char *get_nom_section(const char *shstrtab, Elf32_Shdr sh)
{
  if (!shstrtab || sh.sh_name == 0) return "";
  return shstrtab + sh.sh_name;
}

int section_a_ignorer(const char *name, const Elf32_Shdr *sh)
{
  if (!sh) return 1;

  /* REL / RELA */
  if (sh->sh_type == SHT_REL || sh->sh_type == SHT_RELA) return 1;

  /* debug */
  if (contient_debug(name)) return 1;

  /* on reconstruit .shstrtab  donc on ignore celle de l'entree */
  if (est_shstrtab(name)) return 1;

  return 0;
}

/* ============================================================
 *  Conversion endianness à l’écriture dans le fichier resultat
 * ============================================================ */

void convertir_ehdr_pour_sortie(Elf32_Ehdr *h, int big_endian_out)
{
  if (!big_endian_out) return;

  h->e_type      = reverse_2(h->e_type);
  h->e_machine   = reverse_2(h->e_machine);
  h->e_version   = reverse_4(h->e_version);
  h->e_entry     = reverse_4(h->e_entry);
  h->e_phoff     = reverse_4(h->e_phoff);
  h->e_shoff     = reverse_4(h->e_shoff);
  h->e_flags     = reverse_4(h->e_flags);
  h->e_ehsize    = reverse_2(h->e_ehsize);
  h->e_phentsize = reverse_2(h->e_phentsize);
  h->e_phnum     = reverse_2(h->e_phnum);
  h->e_shentsize = reverse_2(h->e_shentsize);
  h->e_shnum     = reverse_2(h->e_shnum);
  h->e_shstrndx  = reverse_2(h->e_shstrndx);
}

void convertir_shdr_pour_sortie(Elf32_Shdr *s, int big_endian_out)
{
  if (!big_endian_out) return;

  s->sh_name      = reverse_4(s->sh_name);
  s->sh_type      = reverse_4(s->sh_type);
  s->sh_flags     = reverse_4(s->sh_flags);
  s->sh_addr      = reverse_4(s->sh_addr);
  s->sh_offset    = reverse_4(s->sh_offset);
  s->sh_size      = reverse_4(s->sh_size);
  s->sh_link      = reverse_4(s->sh_link);
  s->sh_info      = reverse_4(s->sh_info);
  s->sh_addralign = reverse_4(s->sh_addralign);
  s->sh_entsize   = reverse_4(s->sh_entsize);
}


/* ============================================================
 * Structures internes simplifiées pour construire le résultat
 * ============================================================ */
void vec_init(VecSec *a)
{
  a->v = NULL;
  a->n = 0;
  a->cap = 0;
}
void vec_free(VecSec *a)
{
  if (!a) return;
  for (int i = 0; i < a->n; i++) {
    free(a->v[i].name);
    free(a->v[i].data);
  }
  free(a->v);
  a->v = NULL;
  a->n = 0;
  a->cap = 0;
}

/* Ajoute une section au vecteur. */
int vec_push(VecSec *a, SecR s)
{
  if (a->n == a->cap) {
    int nc = (a->cap == 0) ? 16 : (a->cap * 2);
    SecR *nv = (SecR*)realloc(a->v, (size_t)nc * sizeof(SecR));
    if (!nv) return -1;
    a->v = nv;
    a->cap = nc;
  }
  a->v[a->n++] = s;
  return 0;
}
/* Trouve une section par nom, renvoie son index dans le résultat */
int vec_find_by_name(const VecSec *a, const char *name)
{
  for (int i = 0; i < a->n; i++) {
    if (a->v[i].name && name && strcmp(a->v[i].name, name) == 0) return i;
  }
  return -1;
}

/* ============================================================
 Construction de .shstrtab  table dezs noms pour le fichier résultat
 * ============================================================ */
unsigned char *construire_shstrtab(VecSec *R, uint32_t *out_size)
{
  uint32_t total = 1; /* chaîne vide au debut  le premier octet '\0'*/
  
  for (int i = 1; i < R->n; i++) {
    total += (uint32_t)strlen(R->v[i].name) + 1;
  }

  unsigned char *buf = (unsigned char*)malloc(total);
  if (!buf) return NULL;

  uint32_t off = 0;
  buf[off++] = '\0';
  //pour chaque section i>0 : sh_name = offset dans cette table.
  for (int i = 1; i < R->n; i++) {
    R->v[i].sh.sh_name = off;
    uint32_t len = (uint32_t)strlen(R->v[i].name) + 1;
    memcpy(buf + off, R->v[i].name, len);
    off += len;
  }

  *out_size = total;
  return buf;
}
/* ============================================================
 charger un fichier ELF (header + sections + shstrtab)
 * ============================================================ */

int charger_elf(const char *path,FILE **f_out,Elf32_Ehdr *eh_out,Shdr_liste **L_out,char **shstr_out)
{
  FILE *f = fopen(path, "rb");
  if (!f) { perror("fopen"); return -1; }

  if (E1_read_Elf32_Ehdr(f, eh_out) != 0) { fclose(f); return -1; }

  Shdr_liste *L = (Shdr_liste*)malloc(sizeof(Shdr_liste));
  if (!L) { fclose(f); return -1; }
  L->content = NULL;
  L->next = NULL;

  E2_read_Shdr_list(f, *eh_out, L);

  char *shstr = read_shstrtab(f, *eh_out);
  if (!shstr) { free_Shdr_list(L); fclose(f); return -1; }

  *f_out = f;
  *L_out = L;
  *shstr_out = shstr;
  return 0;
}

/* ========= Construire R[0] = section NULL ========= */
int ajouter_section0(VecSec *R)
{
  SecR s0;
  memset(&s0, 0, sizeof(s0));
  s0.name = dupliquer_chaine("");
  memset(&s0.sh, 0, sizeof(Elf32_Shdr));
  s0.data = NULL;
  s0.data_size = 0;
  if (!s0.name) return -1;
  return vec_push(R, s0);
}

/* =========  Ajouter les sections de A dans R + remplir renumA ========= */
int ajouter_sections_de_A(VecSec *R,Elf32_Ehdr ehA, Shdr_liste *LA,const char *shstrA,uint32_t *renumA)                                                                
{
  renumA[0] = 0; /* section 0 => 0 */

  for (int i = 1; i < (int)ehA.e_shnum; i++) {
    Shdr_liste *sa = section_index(LA, i);////lire le contenu de la section i de A
    if (!sa) { renumA[i] = 0; continue; }

    const char *nameA = get_nom_section(shstrA, sa->header);// recuperer le nom de la section i de A

    if (section_a_ignorer(nameA, &sa->header)) {
      renumA[i] = 0; /* ignorée => pas dans R */
      continue;
    }

    SecR s;
    memset(&s, 0, sizeof(s));
    s.name = dupliquer_chaine(nameA);
    if (!s.name) return -1;

    s.sh = sa->header;//entete de la section
    /* Copier le contenu si ce n’est pas NOBITS et si sh_size > 0 */
    if (s.sh.sh_type != SHT_NOBITS && s.sh.sh_size > 0) {
      s.data_size = s.sh.sh_size;
      s.data = (unsigned char*)malloc(s.data_size);
      if (!s.data) return -1;
      memcpy(s.data, sa->content, s.data_size);
    }

    if (vec_push(R, s) != 0) return -1;

    /* Mapping A : l'index resultat est le dernier ajoute*/
    renumA[i] = (uint32_t)(R->n - 1);
  }

  return 0;
}

/* ========= Traiter B : fusion/ajout + remplir renumB et deltaB ========= */
int traiter_sections_de_B(VecSec *R,Elf32_Ehdr ehB,Shdr_liste *LB,const char *shstrB,uint32_t *renumB,uint32_t *deltaB)                                                                                                                                                           
{
  renumB[0] = 0;
  deltaB[0] = 0;

  for (int i = 1; i < (int)ehB.e_shnum; i++) {
    Shdr_liste *sb = section_index(LB, i);
    if (!sb) { renumB[i] = 0; deltaB[i] = 0; continue; }

    const char *nameB = get_nom_section(shstrB, sb->header);

    if (section_a_ignorer(nameB, &sb->header)) {
      renumB[i] = 0;
      deltaB[i] = 0;
      continue;
    }

    /* .comment : garder une seule occurrence */
    if (est_comment(nameB)) {
      int j = vec_find_by_name(R, nameB);
      if (j >= 0) {
        renumB[i] = (uint32_t)j;
        deltaB[i] = 0;
        continue;
      }
      /* sinon on l'ajoute (comme une section “nouvelle”) */
    }

    int j = vec_find_by_name(R, nameB);

    if (j >= 0) {
      SecR *sr = &R->v[j];//probablement c  la section de A vu qu'on l'a deja ajoutee avant

      /* PROGBITS : concat A||B */
      if (sr->sh.sh_type == SHT_PROGBITS &&
          sb->header.sh_type == SHT_PROGBITS &&
          !est_comment(nameB))
      {
        uint32_t old = sr->sh.sh_size;//au debut elle a size de A
        uint32_t add = sb->header.sh_size;

        renumB[i] = (uint32_t)j;//elle prend le numero de section j dans R car elle est deja presente
        deltaB[i] = old;  /* offset de concat pour les symboles de B dans cette section qui est la taille de section A avant concat */

        if (add > 0) {
          unsigned char *nd = (unsigned char*)realloc(sr->data, old + add);
          if (!nd) return -1;
          sr->data = nd;
          memcpy(sr->data + old, sb->content, add);
          sr->data_size = old + add;
        }
        sr->sh.sh_size = old + add;//nouvelle taille de la sec apres concat
        sr->sh.sh_addralign = (Elf32_Word)max(sr->sh.sh_addralign, sb->header.sh_addralign);
        continue;
      }

      /* NOBITS : additionner (ex .bss) */
      if (sr->sh.sh_type == SHT_NOBITS && sb->header.sh_type == SHT_NOBITS) {
        uint32_t old = sr->sh.sh_size;
        uint32_t add = sb->header.sh_size;

        renumB[i] = (uint32_t)j;
        deltaB[i] = old;

        sr->sh.sh_size = old + add;
        sr->sh.sh_addralign = (Elf32_Word)max(sr->sh.sh_addralign, sb->header.sh_addralign);
        continue;
      }

      /* Autres types : on mappe B vers la section existante */
      renumB[i] = (uint32_t)j;
      deltaB[i] = 0;
      continue;
    }

    /* Section absente : ajout telle quelle */
    {
      SecR s;
      memset(&s, 0, sizeof(s));
      s.name = dupliquer_chaine(nameB);
      if (!s.name) return -1;

      s.sh = sb->header;

      if (s.sh.sh_type != SHT_NOBITS && s.sh.sh_size > 0) {
        s.data_size = s.sh.sh_size;
        s.data = (unsigned char*)malloc(s.data_size);
        if (!s.data) return -1;
        memcpy(s.data, sb->content, s.data_size);
      }

      if (vec_push(R, s) != 0) return -1;

      renumB[i] = (uint32_t)(R->n - 1);
      deltaB[i] = 0;//pas de concat donc delta=0 pas d'offset a ajouter aux symboles
    }
  }

  return 0;
}

/* =========Ajouter + reconstruire .shstrtab ========= */
int ajouter_shstrtab(VecSec *R, int *shstrndx_out)
{
  int shstrndx = R->n;//index de la section .shstrtab dans R

  {
    SecR s;
    memset(&s, 0, sizeof(s));
    s.name = dupliquer_chaine(".shstrtab");
    if (!s.name) return -1;

    memset(&s.sh, 0, sizeof(Elf32_Shdr));
    s.sh.sh_type = SHT_STRTAB;
    s.sh.sh_addralign = 1;

    if (vec_push(R, s) != 0) return -1;
  }

  /* Construire le contenu et remplir sh_name */
  uint32_t shstr_size = 0;
  unsigned char *shstr_data = construire_shstrtab(R, &shstr_size);
  if (!shstr_data) return -1;

  R->v[shstrndx].data = shstr_data;
  R->v[shstrndx].data_size = shstr_size;
  R->v[shstrndx].sh.sh_size = shstr_size;

  *shstrndx_out = shstrndx;
  return 0;
}

/* ========= Correction minimale (sans toucher aux symboles) =========*/
     /* --- Correction minimale : sh_link de .symtab doit pointer vers .strtab et sera traité dans l'etape 7 --- 
  readelf: Warning: Section 5 has an out of range sh_link value of 17*/
void correction_minimale_sh_link(VecSec *R)
{
  int idx_symtab = vec_find_by_name(R, ".symtab");
  int idx_strtab = vec_find_by_name(R, ".strtab");
  if (idx_symtab >= 0 && idx_strtab >= 0) {
    R->v[idx_symtab].sh.sh_link = (Elf32_Word)idx_strtab;
  }
}


/* ========= Ecriture ELF  ========= */
int ecrire_elf_resultat(const char *fileOut,const Elf32_Ehdr *eh_base,VecSec *R,int shstrndx,int out_big)                             
{
  FILE *fo = fopen(fileOut, "wb");
  if (!fo) { perror("fopen out"); return -1; }

  /* Preparer un Ehdr res basé sur A */
  Elf32_Ehdr out = *eh_base;
  out.e_type = ET_REL;
  out.e_entry = 0;

  out.e_phoff = 0;
  out.e_phnum = 0;
  out.e_phentsize = 0;

  out.e_ehsize = sizeof(Elf32_Ehdr);
  out.e_shentsize = sizeof(Elf32_Shdr);
  out.e_shnum = (Elf32_Half)R->n;
  out.e_shstrndx = (Elf32_Half)shstrndx;
  out.e_shoff = 0; /* fixé après placement */

  /* ecrire enttete provisoire */
  {
    Elf32_Ehdr tmp = out;
    convertir_ehdr_pour_sortie(&tmp, out_big);
    if (fwrite(&tmp, 1, sizeof(tmp), fo) != sizeof(tmp)) { fclose(fo); return -1; }
  }

  /* Placer sections + ecrire contenu */
  uint32_t cursor = (uint32_t)sizeof(Elf32_Ehdr);

  for (int i = 1; i < R->n; i++) {
    SecR *s = &R->v[i];

    cursor = align_up(cursor, (s->sh.sh_addralign ? s->sh.sh_addralign : 1));

    while ((uint32_t)ftell(fo) < cursor) fputc(0, fo);

    s->sh.sh_offset = cursor;

    if (s->sh.sh_type == SHT_NOBITS) {
      /* NOBITS : pas d’octets écrits */
      continue;
    }

    if (s->sh.sh_size > 0) {
      if (!s->data) { fclose(fo); return -1; }
      if (fwrite(s->data, 1, s->sh.sh_size, fo) != s->sh.sh_size) { fclose(fo); return -1; }
      cursor += s->sh.sh_size;
    }
  }

  /* ecrire table des sections à la fin */
  cursor = align_up(cursor, 4);
  while ((uint32_t)ftell(fo) < cursor) fputc(0, fo);

  out.e_shoff = cursor;

  /* reéécrire l’entete final */
  if (fseek(fo, 0, SEEK_SET) != 0) { fclose(fo); return -1; }
  {
    Elf32_Ehdr tmp = out;
    convertir_ehdr_pour_sortie(&tmp, out_big);
    if (fwrite(&tmp, 1, sizeof(tmp), fo) != sizeof(tmp)) { fclose(fo); return -1; }
  }

  /* Écrire les Shdr */
  if (fseek(fo, out.e_shoff, SEEK_SET) != 0) { fclose(fo); return -1; }
  for (int i = 0; i < R->n; i++) {
    Elf32_Shdr tmp = R->v[i].sh;
    convertir_shdr_pour_sortie(&tmp, out_big);
    if (fwrite(&tmp, 1, sizeof(tmp), fo) != sizeof(tmp)) { fclose(fo); return -1; }
  }

  fclose(fo);
  return 0;
}








/* ============================================================
 *  fusion Étape 6
 * ============================================================ */
int E6_fusionner_sections(const char *fileA, const char *fileB, const char *fileOut,uint32_t **renumA_out, size_t *lenA_out,uint32_t **renumB_out, uint32_t **deltaB_out, size_t *lenB_out)
{
  FILE *fa = NULL, *fb = NULL;
  Elf32_Ehdr ehA, ehB;
  Shdr_liste *LA = NULL, *LB = NULL;
  char *shstrA = NULL, *shstrB = NULL;

  uint32_t *renumA = NULL;
  uint32_t *renumB = NULL;
  uint32_t *deltaB = NULL;

  VecSec R;
  vec_init(&R);

  /* Charger A */
  if (charger_elf(fileA, &fa, &ehA, &LA, &shstrA) != 0) goto fail;

  renumA = (uint32_t*)calloc(ehA.e_shnum, sizeof(uint32_t));
  if (!renumA) goto fail;

  /* Charger B */
  if (charger_elf(fileB, &fb, &ehB, &LB, &shstrB) != 0) goto fail;

  renumB = (uint32_t*)calloc(ehB.e_shnum, sizeof(uint32_t));
  deltaB = (uint32_t*)calloc(ehB.e_shnum, sizeof(uint32_t));
  if (!renumB || !deltaB) goto fail;

  /* R[0] = NULL */
  if (ajouter_section0(&R) != 0) goto fail;

  /* Ajouter sections A + remplir renumA */
  if (ajouter_sections_de_A(&R, ehA, LA, shstrA, renumA) != 0) goto fail;

  /*  Traiter sections B + remplir renumB/deltaB */
  if (traiter_sections_de_B(&R, ehB, LB, shstrB, renumB, deltaB) != 0) goto fail;

  /*  Ajouter .shstrtab reconstruit */
  int shstrndx = -1;
  if (ajouter_shstrtab(&R, &shstrndx) != 0) goto fail;

  /*  Correction minimale (sans toucher aux symboles) */
  correction_minimale_sh_link(&R);

  /*  Ecriture ELF (fonction dédiée) */
  int out_big = is_big_endian_fich(ehA);
  if (ecrire_elf_resultat(fileOut, &ehA, &R, shstrndx, out_big) != 0) goto fail;

  /*  Renvoyer les mappings */
  if (renumA_out) *renumA_out = renumA; else free(renumA);
  if (lenA_out)   *lenA_out = (size_t)ehA.e_shnum;

  if (renumB_out) *renumB_out = renumB; else free(renumB);
  if (deltaB_out) *deltaB_out = deltaB; else free(deltaB);
  if (lenB_out)   *lenB_out = (size_t)ehB.e_shnum;

  /* Libérations */
  if (fa) fclose(fa);
  if (fb) fclose(fb);
  free(shstrA);
  free(shstrB);
  free_Shdr_list(LA);
  free_Shdr_list(LB);
  vec_free(&R);
  return 0;

fail:
  if (fa) fclose(fa);
  if (fb) fclose(fb);
  free(shstrA);
  free(shstrB);
  if (LA) free_Shdr_list(LA);
  if (LB) free_Shdr_list(LB);
  vec_free(&R);
  free(renumA);
  free(renumB);
  free(deltaB);
  return -1;
}


/****************************************************************************************************** */
/****************************************************************************************************** */
/****************************************************************************************************** */
/****************************************************************************************************** */
/****************************************************************************************************** */


int sym_est_local(const Elf32_Sym *s) {
  return ELF32_ST_BIND(s->st_info) == STB_LOCAL;
}

int sym_est_undef(const Elf32_Sym *s) {
  return s->st_shndx == SHN_UNDEF;
}

int sym_est_special(const Elf32_Sym *s) {
  return s->st_shndx == SHN_UNDEF || s->st_shndx == SHN_ABS || s->st_shndx == SHN_COMMON;
}

int sym_est_defini_en_section(const Elf32_Sym *s) {
  return !sym_est_special(s);
}

/* Si la section d’un symbole on l'a ignorée dans OUT (renum==0), on peut jeter le symbole */
int sym_pointe_section_ignorée(const Elf32_Sym *s, const uint32_t *renum, size_t renum_len) {
  if (sym_est_special(s)) return 0;
  if (s->st_shndx >= renum_len) return 1;
  return (renum[s->st_shndx] == 0);
}

/* ---------- Lecture symtab/strtab d’un fichier ELF ---------- */


void free_table_sym(TableSym *t) {
  if (!t) return;
  free(t->syms);
  free(t->strtab);
  memset(t, 0, sizeof(*t));
}

/* Trouver une section par nom dans la Shdr_liste (en utilisant shstrtab) */
Shdr_liste* trouver_section_par_nom(Shdr_liste *L, const char *shstr, const char *nom) {
  if (!L || !shstr || !nom) return NULL;
  for (Shdr_liste *p = L; p; p = p->next) {
    const char *n = shstr + p->header.sh_name;
    if (strcmp(n, nom) == 0) return p;
  }
  return NULL;
}

/* Lire un bloc ou section depuis la liste  */
int charger_table_sym(FILE *f, Elf32_Ehdr eh, Shdr_liste *L, char *shstr, TableSym *out) {
  memset(out, 0, sizeof(*out));

  Shdr_liste *sec_sym = trouver_section_par_nom(L, shstr, ".symtab");
  if (!sec_sym) return -1;

  /* .strtab associée = section index sec_sym->sh_link */
  int idx_str = (int)sec_sym->header.sh_link;
  Shdr_liste *sec_str = section_index(L, idx_str);
  if (!sec_str) return -1;

  /* symtab */
  if (sec_sym->header.sh_entsize == 0) return -1;
  out->nsyms = sec_sym->header.sh_size / sec_sym->header.sh_entsize;
  out->syms  = (Elf32_Sym*)malloc(out->nsyms * sizeof(Elf32_Sym));
  if (!out->syms) return -1;
  memcpy(out->syms, sec_sym->content, out->nsyms * sizeof(Elf32_Sym));

  /* strtab */
  out->strsz  = sec_str->header.sh_size;
  out->strtab = (char*)malloc(out->strsz);
  if (!out->strtab) return -1;
  memcpy(out->strtab, sec_str->content, out->strsz);

  /* Endianness corriger les Elf32_Sym */
  if (is_big_endian_fich(eh)) {
    for (size_t i = 0; i < out->nsyms; i++) correct_endian_sym(&out->syms[i]);
  }

  return 0;
}

/* ---------- Construction d'une nouvelle strtab ---------- */
int buf_init(Buf *b) {
  b->cap = 256;
  b->sz  = 0;
  b->data = (unsigned char*)malloc(b->cap);
  if (!b->data) return -1;
  b->data[b->sz++] = '\0'; /* strtab commence toujours par 0 */
  return 0;
}

void buf_free(Buf *b) {
  free(b->data);
  memset(b, 0, sizeof(*b));
}

int buf_reserve(Buf *b, size_t need) {
  if (need <= b->cap) return 0;
  size_t ncap = b->cap;
  while (ncap < need) ncap *= 2;
  unsigned char *nd = (unsigned char*)realloc(b->data, ncap);
  if (!nd) return -1;
  b->data = nd;
  b->cap = ncap;
  return 0;
}

uint32_t strtab_ajouter(Buf *b, const char *s) {
  if (!s) s = "";
  size_t n = strlen(s) + 1;
  size_t off = b->sz;
  if (buf_reserve(b, b->sz + n) != 0) return 0;
  memcpy(b->data + b->sz, s, n);
  b->sz += n;
  return (uint32_t)off;
}

/* ---------- Table de symboles OUT ---------- */
int vecsym_init(VecSym *vs) {
  vs->cap = 64;
  vs->n = 0;
  vs->v = (Elf32_Sym*)malloc(vs->cap * sizeof(Elf32_Sym));
  return vs->v ? 0 : -1;
}

void vecsym_free(VecSym *vs) {
  free(vs->v);
  memset(vs, 0, sizeof(*vs));
}

int vecsym_push(VecSym *vs, const Elf32_Sym *s) {
  if (vs->n == vs->cap) {
    size_t ncap = vs->cap * 2;
    Elf32_Sym *nv = (Elf32_Sym*)realloc(vs->v, ncap * sizeof(Elf32_Sym));
    if (!nv) return -1;
    vs->v = nv;
    vs->cap = ncap;
  }
  vs->v[vs->n++] = *s;
  return 0;
}

/* Recherche  d’un symbole global déja ajoute*/
int trouver_global_par_nom(const VecSym *out, const char *out_strtab, const char *name) {
  for (size_t i = 0; i < out->n; i++) {
    if (sym_est_local(&out->v[i])) continue;
    const char *n = out_strtab + out->v[i].st_name;
    if (strcmp(n, name) == 0) return (int)i;
  }
  return -1;
}

int corriger_symbole_pour_out(Elf32_Sym *dst,const Elf32_Sym *src,const uint32_t *renumSec, size_t renumSec_len,const uint32_t *deltaSec ,const char *src_strtab,Buf *out_strtab)
{
  //deltaSec  NULL pour A car pas de decalage a appliquer
  *dst = *src;

  const char *name = src_strtab + src->st_name;
  dst->st_name = strtab_ajouter(out_strtab, name);

  if (sym_est_special(src)) {
    /* UND/ABS/COMMON : rien à renumerer */
    return 0;
  }

  if (src->st_shndx >= renumSec_len) return -1;
  uint32_t new_sh = renumSec[src->st_shndx];
  if (new_sh == 0) return -2; /* section ignorée -> caller décidera (souvent: skip) */

  dst->st_shndx = (Elf32_Half)new_sh;

  if (deltaSec) {
    dst->st_value += deltaSec[src->st_shndx];
  }
  return 0;
}

/* ============================================================
 *  Fusion, renumerotation et correction des symboles
 * ============================================================ */
/*construire VecSec depuis liste pour avoir le format voulu lors de l'écriture*/
int construire_vecsec_depuis_liste(VecSec *R, Shdr_liste *L, const char *shstrtab)
{
  vec_init(R);

  for (Shdr_liste *p = L; p != NULL; p = p->next) {
    SecR s;
    memset(&s, 0, sizeof(s));

    /* nom de section */
    const char *nm = "";
    if (shstrtab && p->header.sh_name != 0)
      nm = shstrtab + p->header.sh_name;

    s.name = dupliquer_chaine(nm);
    if (!s.name) { vec_free(R); return -1; }

    s.sh = p->header;

    /* contenu */
    if (s.sh.sh_type != SHT_NOBITS && s.sh.sh_size > 0) {
      s.data_size = s.sh.sh_size;
      s.data = (unsigned char*)malloc(s.data_size);
      if (!s.data) { free(s.name); vec_free(R); return -1; }
      memcpy(s.data, p->content, s.data_size);
    }

    if (vec_push(R, s) != 0) {
      free(s.data);
      free(s.name);
      vec_free(R);
      return -1;
    }
  }

  return 0;
}
/*******************pour afficher le nom de la section aussi dans la table des symboles *************************/
static const char *secname_by_index(const Shdr_liste *Lout, const char *shstrOut, uint32_t shndx)
{
  if (!Lout || !shstrOut) return "";
  const Shdr_liste *p = Lout;
  uint32_t i = 0;
  while (p && i < shndx) { p = p->next; i++; }
  if (!p) return "";
  return shstrOut + p->header.sh_name;
}


static void section_symbol_name(Elf32_Sym *dst, Buf *outStr, const Shdr_liste *Lout, const char *shstrOut)
{
  if (ELF32_ST_TYPE(dst->st_info) != STT_SECTION) return;
  const char *nm = secname_by_index(Lout, shstrOut, (uint32_t)dst->st_shndx);
  dst->st_name = strtab_ajouter(outStr, nm);
}



int E7_fusionner_corriger_symboles(const char *fileA,
                                  const char *fileB,
                                  const char *fileOut6,
                                  const char *fileOut7,
                                  const uint32_t *renumA, size_t lenA,
                                  const uint32_t *renumB, const uint32_t *deltaB, size_t lenB,
                                  uint32_t **renumSymA_out, size_t *nSymA_out,
                                  uint32_t **renumSymB_out, size_t *nSymB_out)
{
  int ret = -1;// valeur de retour par défaut = erreur

  FILE *fa = NULL, *fb = NULL, *fin = NULL;// fichiers A, B et OUT6
  Elf32_Ehdr ehA, ehB, ehOut;// entetes ELF

  Shdr_liste *LA = NULL, *LB = NULL, *Lout = NULL;// listes des sections  A B out6
  char *shstrA = NULL, *shstrB = NULL, *shstrOut = NULL;// shstrtabs

  TableSym TA, TB;// tables des symboles A et B
  memset(&TA, 0, sizeof(TA));
  memset(&TB, 0, sizeof(TB));

  uint32_t *renumSymA = NULL, *renumSymB = NULL;// renumerotation des symboles A et B

  Buf outStr;// nouvelle strtab
  VecSym outSym;// nouvelle symtab
  int outStr_init = 0, outSym_init = 0;

  VecSec R;// sections résultat
  int R_init = 0;

  /* =========  Charger A et B (symtabs) ========= */
  if (charger_elf(fileA, &fa, &ehA, &LA, &shstrA) != 0) goto done;
  if (charger_table_sym(fa, ehA, LA, shstrA, &TA) != 0) goto done;

  if (charger_elf(fileB, &fb, &ehB, &LB, &shstrB) != 0) goto done;
  if (charger_table_sym(fb, ehB, LB, shstrB, &TB) != 0) goto done;

  renumSymA = (uint32_t*)calloc(TA.nsyms, sizeof(uint32_t));
  renumSymB = (uint32_t*)calloc(TB.nsyms, sizeof(uint32_t));
  if (!renumSymA || !renumSymB) goto done;

  /* =========  Charger out6 (sections) ========= */
  if (charger_elf(fileOut6, &fin, &ehOut, &Lout, &shstrOut) != 0) goto done;

  /* =========  Construire la nouvelle .symtab + .strtab ========= */
  if (buf_init(&outStr) != 0) goto done;
  outStr_init = 1;//indicateur d'initialisation
  if (vecsym_init(&outSym) != 0) goto done;
  outSym_init = 1;

  /* entrée 0 obligatoire */
  {
    Elf32_Sym s0; memset(&s0, 0, sizeof(s0));
    if (vecsym_push(&outSym, &s0) != 0) goto done;
  }

  /* ---- Locaux : A puis B ---- */
  for (size_t i = 1; i < TA.nsyms; i++) {
    if (!sym_est_local(&TA.syms[i])) continue;

    if (sym_pointe_section_ignorée(&TA.syms[i], renumA, lenA)) {
      renumSymA[i] = 0;
      continue;
    }

    Elf32_Sym dst;
    int cr = corriger_symbole_pour_out(&dst, &TA.syms[i], renumA, lenA, NULL, TA.strtab, &outStr);//pas de decalage pour A
    if (cr == -2) { renumSymA[i] = 0; continue; }// section ignorée
    if (cr != 0) goto done;
    section_symbol_name(&dst, &outStr, Lout, shstrOut);

    renumSymA[i] = (uint32_t)outSym.n;// nouvelle position du symbole dans outSym
    if (vecsym_push(&outSym, &dst) != 0) goto done;
  }

  for (size_t i = 1; i < TB.nsyms; i++) {
    if (!sym_est_local(&TB.syms[i])) continue;

    if (sym_pointe_section_ignorée(&TB.syms[i], renumB, lenB)) {
      renumSymB[i] = 0;
      continue;
    }

    Elf32_Sym dst;
    int cr = corriger_symbole_pour_out(&dst, &TB.syms[i], renumB, lenB, deltaB, TB.strtab, &outStr);//section de B avec decalage
    if (cr == -2) { renumSymB[i] = 0; continue; }
    if (cr != 0) goto done;
    section_symbol_name(&dst, &outStr, Lout, shstrOut);

    renumSymB[i] = (uint32_t)outSym.n;// nouvelle position du symbole dans outSym
    if (vecsym_push(&outSym, &dst) != 0) goto done;
  }

  uint32_t first_global = (uint32_t)outSym.n;

  /* ---- 3.b Globaux : A puis B avec règles ---- */
  for (size_t i = 1; i < TA.nsyms; i++) {
    if (sym_est_local(&TA.syms[i])) continue;

    const char *name = TA.strtab + TA.syms[i].st_name;
    int pos = trouver_global_par_nom(&outSym, (char*)outStr.data, name);
   
    if (pos < 0) {
       /* pas encore présent -> ajouter */
      if (sym_pointe_section_ignorée(&TA.syms[i], renumA, lenA)) { renumSymA[i] = 0; continue; }

      Elf32_Sym dst;
      int cr = corriger_symbole_pour_out(&dst, &TA.syms[i], renumA, lenA, NULL, TA.strtab, &outStr);
      if (cr == -2) { renumSymA[i] = 0; continue; }
      if (cr != 0) goto done;

      renumSymA[i] = (uint32_t)outSym.n;
      if (vecsym_push(&outSym, &dst) != 0) goto done;
    } else {
      /* déjà présent : on ne fait rien ici (c'est A qui a cree l’entrée) */
      renumSymA[i] = (uint32_t)pos;
    }
  }

  for (size_t i = 1; i < TB.nsyms; i++) {
    if (sym_est_local(&TB.syms[i])) continue;

    const char *name = TB.strtab + TB.syms[i].st_name;
    int pos = trouver_global_par_nom(&outSym, (char*)outStr.data, name);

    if (pos < 0) {
      if (sym_pointe_section_ignorée(&TB.syms[i], renumB, lenB)) { renumSymB[i] = 0; continue; }

      Elf32_Sym dst;
      int cr = corriger_symbole_pour_out(&dst, &TB.syms[i], renumB, lenB, deltaB, TB.strtab, &outStr);
      if (cr == -2) { renumSymB[i] = 0; continue; }
      if (cr != 0) goto done;

      renumSymB[i] = (uint32_t)outSym.n;
      if (vecsym_push(&outSym, &dst) != 0) goto done;
    } else {
      /* déjà présent : fusion selon règles */
      Elf32_Sym *exist = &outSym.v[pos];
      int exist_undef = (exist->st_shndx == SHN_UNDEF);
      int new_undef   = sym_est_undef(&TB.syms[i]);
      // les deux définis -> erreur
      if (!exist_undef && !new_undef) {
        fprintf(stderr, "E7 ERREUR: symbole global '%s' défini dans A et B\n", name);
        goto done;
      }
      /* si exist UND et B défini => remplacer par la définition de B */
      if (exist_undef && !new_undef) {
        Elf32_Sym dst;
        int cr = corriger_symbole_pour_out(&dst, &TB.syms[i], renumB, lenB, deltaB, TB.strtab, &outStr);
        if (cr == -2) { renumSymB[i] = 0; continue; }
        if (cr != 0) goto done;
        dst.st_name = exist->st_name;
        *exist = dst; /* remplace l'UND par la définition */
      }
      /* mettre à jour la renumération */
      renumSymB[i] = (uint32_t)pos;
    }
  }

  /* ========= 4) Remplacer .symtab / .strtab dans Lout6========= */
  Shdr_liste *sec_sym_out = trouver_section_par_nom(Lout, shstrOut, ".symtab");
  Shdr_liste *sec_str_out = trouver_section_par_nom(Lout, shstrOut, ".strtab");
  if (!sec_sym_out || !sec_str_out) goto done;

  /* .strtab */
  free(sec_str_out->content);
  sec_str_out->content = (unsigned char*)malloc(outStr.sz);
  if (!sec_str_out->content) goto done;
  memcpy(sec_str_out->content, outStr.data, outStr.sz);
  sec_str_out->header.sh_size = (Elf32_Word)outStr.sz;
  sec_str_out->header.sh_type = SHT_STRTAB;
  sec_str_out->header.sh_addralign = 1;

  /* .symtab */
  int out_big = is_big_endian_fich(ehOut);
  free(sec_sym_out->content);
  sec_sym_out->content = (unsigned char*)malloc(outSym.n * sizeof(Elf32_Sym));
  if (!sec_sym_out->content) goto done;
  /* Copie temporaire pour conversion endian sans casser outSym.v */
  Elf32_Sym *tmp = (Elf32_Sym*)malloc(outSym.n * sizeof(Elf32_Sym));
  if (!tmp) goto done;

  memcpy(tmp, outSym.v, outSym.n * sizeof(Elf32_Sym));

  /* Convertir les champs multi-octets des Elf32_Sym si sortie big-endian */
  if (out_big) {
    for (size_t k = 0; k < outSym.n; k++) {
      correct_endian_sym(&tmp[k]);
    }
  }

  memcpy(sec_sym_out->content, tmp, outSym.n * sizeof(Elf32_Sym));
  free(tmp);

  //memcpy(sec_sym_out->content, outSym.v, outSym.n * sizeof(Elf32_Sym));
  sec_sym_out->header.sh_size = (Elf32_Word)(outSym.n * sizeof(Elf32_Sym));
  sec_sym_out->header.sh_type = SHT_SYMTAB;
  sec_sym_out->header.sh_entsize = sizeof(Elf32_Sym);
  sec_sym_out->header.sh_info = (Elf32_Word)first_global;

  /* ========= 5) Re-écriture out7 via VecSec + ecrire_elf_resultat ========= */
  if (construire_vecsec_depuis_liste(&R, Lout, shstrOut) != 0) goto done;
  R_init = 1;

  if (ecrire_elf_resultat(fileOut7, &ehOut, &R, (int)ehOut.e_shstrndx, is_big_endian_fich(ehOut)) != 0)
    goto done;

  /* ========= Sorties ========= */
  if (renumSymA_out) *renumSymA_out = renumSymA; else free(renumSymA);
  if (renumSymB_out) *renumSymB_out = renumSymB; else free(renumSymB);
  if (nSymA_out) *nSymA_out = TA.nsyms;
  if (nSymB_out) *nSymB_out = TB.nsyms;

  ret = 0;

done:
  if (ret != 0) { free(renumSymA); free(renumSymB); }

  if (fa) fclose(fa);
  if (fb) fclose(fb);
  if (fin) fclose(fin);

  free(shstrA); free(shstrB); free(shstrOut);
  if (LA) free_Shdr_list(LA);
  if (LB) free_Shdr_list(LB);
  if (Lout) free_Shdr_list(Lout);

  free_table_sym(&TA);
  free_table_sym(&TB);

  if (outSym_init) vecsym_free(&outSym);
  if (outStr_init) buf_free(&outStr);

  if (R_init) vec_free(&R);

  return ret;
}




/***************************************************************************************
 * ****************************************E8 *****************************************************************************
 ***************************************************************************************/
 /*reconstruire les .rel.* en corrigeant  l’offset ou patcher et l’index du symbole référencé, en utilisant les  mappings de E6 (renum/delta) + E7 (renumSym).*/
 /*
    -Lire les sections .rel.* des deux fichiers 
    -Pour tout Elf32_Rel : -corrige r_offset si ca vien de B et qur la section cible a été concaténée
                          -corrige r_info pour le symbole référencé
    -Recréer les sections .rel.* dans le fichier de sortie
    -Ecrire le fichier de sortie out8.o
    */

  // Corriger l’endianess d’un Elf32_Rel
static void correct_endian_rel(Elf32_Rel *r) {
  r->r_offset = reverse_4(r->r_offset);
  r->r_info   = reverse_4(r->r_info);
}

  // Constuire le nom ".rel.<nom_section>"
static char *make_rel_name(const char *target_sec_name) {
  if (!target_sec_name) target_sec_name = "";
  size_t n = strlen(".rel") + strlen(target_sec_name) + 1;
  char *s = (char*)malloc(n);
  if (!s) return NULL;
  snprintf(s, n, ".rel%s", target_sec_name);
  return s;
}

  // Buffer dynamique Stockant pour Elf32_Rel
// facilite l’ajout dynamique
static int relagg_push(RelAgg *a, const Elf32_Rel *r) {
  if (a->n == a->cap) {
    size_t nc = (a->cap == 0) ? 64 : (a->cap * 2);
    Elf32_Rel *nr = (Elf32_Rel*)realloc(a->rels, nc * sizeof(Elf32_Rel));
    if (!nr) return -1;
    a->rels = nr;
    a->cap = nc;
  }
  a->rels[a->n++] = *r;
  return 0;
}

  // Trouver une section par nom sinon la creer dans R
static int find_or_create_relagg(RelAgg **arr, size_t *n, size_t *cap,
                                 const char *relname, int target_out_index)
{
  for (size_t i = 0; i < *n; i++) {
    if (strcmp((*arr)[i].name, relname) == 0) return (int)i;
  }

  if (*n == *cap) {
    size_t nc = (*cap == 0) ? 8 : (*cap * 2);
    RelAgg *na = (RelAgg*)realloc(*arr, nc * sizeof(RelAgg));
    if (!na) return -1;
    *arr = na;
    *cap = nc;
  }

  RelAgg *a = &(*arr)[(*n)++];
  memset(a, 0, sizeof(*a));
  a->name = strdup(relname);
  a->target_out_index = target_out_index;
  return (int)(*n - 1);
}

static void free_relaggs(RelAgg *arr, size_t n) {
  for (size_t i = 0; i < n; i++) {
    free(arr[i].name);
    free(arr[i].rels);
  }
  free(arr);
}


// Fonction parcourt les sections selectionne les SHT_REL et corrige les Elf32_Rel

static int absorber_relocations(const Elf32_Ehdr *eh,
                                Shdr_liste *L,
                                const char *shstr,
                                const uint32_t *renumSec, size_t lenSec,
                                const uint32_t *deltaSec,// NULL pour A
                                const uint32_t *renumSym, size_t lenSym,
                                VecSec *Rout, int out_big,
                                RelAgg **aggs, size_t *naggs, size_t *capaggs)
{

  int file_big = is_big_endian_fich(*eh);


  /* Parcourir toutes les sections du fichier */
  for (int i = 1; i < (int)eh->e_shnum; i++) {
    Shdr_liste *sec = section_index(L, i);
    if (!sec) continue;

    /* On ne traite que les sections de type SHT_REL */
    if (sec->header.sh_type != SHT_REL) continue;

    /* Récupérer l'index de la section cible (sh_info) */
    uint32_t target_old = sec->header.sh_info;
    if (target_old >= lenSec) continue;

    /* Renumeroter la section cible */
    uint32_t target_new = renumSec[target_old];
    if (target_new == 0) continue; /* section cible ignorée dans OUT */

    /* Récupérer le nom de la section cible pour nommer .rel.<cible> */
    Shdr_liste *target_sec = section_index(L, (int)target_old);
    if (!target_sec) continue;

    const char *target_name = (shstr ? (shstr + target_sec->header.sh_name) : "");
    char *relname = make_rel_name(target_name);
    if (!relname) return -1;

    /* Trouver ou créer l'agrégat correspondant */
    int idx = find_or_create_relagg(aggs, naggs, capaggs, relname, (int)target_new);
    free(relname);
    if (idx < 0) return -1;

    /* Lire les entrées de relocation */
    size_t entsize = sec->header.sh_entsize ? sec->header.sh_entsize : sizeof(Elf32_Rel);
    size_t nrel = (entsize ? (sec->header.sh_size / entsize) : 0);
    if (nrel == 0) continue;

    Elf32_Rel *tab = (Elf32_Rel*)sec->content;
    if (!tab) return -1;

    /* Traiter chaque relocation */
    for (size_t k = 0; k < nrel; k++) {
      Elf32_Rel r = tab[k];

      /* Remettre en endian host pour calculer correctement */
      if (file_big) correct_endian_rel(&r);

      /* 1) Corriger r_offset si section concaténée (deltaSec != NULL) */
      if (deltaSec) {
        r.r_offset += deltaSec[target_old];
      }

      /* 2) Corriger le symbole dans r_info */
      uint32_t oldSym = ELF32_R_SYM(r.r_info);
      uint32_t type   = ELF32_R_TYPE(r.r_info);

      uint32_t newSym;

      /* FIX CRITIQUE : Traiter le symbole 0 séparément
       * Le symbole index 0 (NULL) est VALIDE pour certaines relocations
       * comme R_ARM_V4BX. Il ne doit PAS être skip 
       */
      if (oldSym == 0) {
        /* Symbole NULL - garder tel quel */
        newSym = 0;
      } else {
        /* Symbole normal (index >= 1) */
        if (oldSym >= lenSym) {
          /* Symbole invalide => skip cette relocation */
          continue;
        }

        newSym = renumSym[oldSym];
        if (newSym == 0) {
          /* Symbole supprimé/ignoré => skip cette relocation */
          continue;
        }
      }

      /* Reconstruire r_info avec le nouveau symbole */
      r.r_info = ELF32_R_INFO(newSym, type);

      /* 3: correction de l'addend pour STT_SECTION (REL) === */
      //on décale déjà les symboles de B avec dst->st_value += deltaSec[src->st_shndx] dans E7
      //donc pas besoin de le faire ici pour les relocations sur des symboles de type STT_SECTION 
      //ça met son st_value au bon début (offset = delta).
      /* Stocker dans l'agrégat (en endian host pour l'instant) */
      if (relagg_push(&(*aggs)[idx], &r) != 0) return -1;
    }
  }

  return 0;
}


int E8_fusionner_corriger_relocations(const char *fileA,
                                      const char *fileB,
                                      const char *fileOut7,
                                      const char *fileOut8,
                                      const uint32_t *renumA, size_t lenA,
                                      const uint32_t *renumB, const uint32_t *deltaB, size_t lenB,
                                      const uint32_t *renumSymA, size_t nSymA,
                                      const uint32_t *renumSymB, size_t nSymB)
{
  int ret = -1;

  FILE *fa=NULL,*fb=NULL,*fo7=NULL;
  Elf32_Ehdr ehA, ehB, ehO;

  Shdr_liste *LA=NULL,*LB=NULL,*LO=NULL;
  char *shstrA=NULL,*shstrB=NULL,*shstrO=NULL;

  VecSec R; vec_init(&R);

  RelAgg *aggs = NULL;
  size_t naggs = 0, capaggs = 0;

  /* Charger A, B, OUT7 */
  if (charger_elf(fileA, &fa, &ehA, &LA, &shstrA) != 0) goto done;
  if (charger_elf(fileB, &fb, &ehB, &LB, &shstrB) != 0) goto done;
  if (charger_elf(fileOut7, &fo7, &ehO, &LO, &shstrO) != 0) goto done;
  int out_big = is_big_endian_fich(ehO);
  /* R[0] = NULL section */
  if (ajouter_section0(&R) != 0) goto done;

  /* Copier toutes les sections de OUT7 dans R (sauf .shstrtab qu’on reconstruit) */
  for (int i = 1; i < (int)ehO.e_shnum; i++) {
    Shdr_liste *s = section_index(LO, i);
    if (!s) continue;

    const char *nm = (shstrO ? shstrO + s->header.sh_name : "");
    if (nm && strcmp(nm, ".shstrtab") == 0) continue; /* on reconstruit */

    SecR out;
    memset(&out, 0, sizeof(out));
    out.name = dupliquer_chaine(nm);
    if (!out.name) goto done;

    out.sh = s->header;

    if (out.sh.sh_type != SHT_NOBITS && out.sh.sh_size > 0) {
      out.data_size = out.sh.sh_size;
      out.data = (unsigned char*)malloc(out.data_size);
      if (!out.data) { free(out.name); goto done; }
      memcpy(out.data, s->content, out.data_size);
    }

    if (vec_push(&R, out) != 0) goto done;
  }

  /* Absorber relocations de A */
  if (absorber_relocations(&ehA, LA, shstrA,
                           renumA, lenA,
                           NULL,// pas de delta pour A
                           renumSymA, nSymA,
                           &R, out_big,
                           &aggs, &naggs, &capaggs) != 0) goto done;

  /* Absorber relocations de B (deltaB important !) */
  if (absorber_relocations(&ehB, LB, shstrB,
                           renumB, lenB,
                           deltaB,
                           renumSymB, nSymB,
                           &R, out_big,
                           &aggs, &naggs, &capaggs) != 0) goto done;

  /* Ajouter les sections .rel.* dans R */
  for (size_t i = 0; i < naggs; i++) {
    RelAgg *a = &aggs[i];

    SecR s;
    memset(&s, 0, sizeof(s));

    s.name = dupliquer_chaine(a->name);
    if (!s.name) goto done;

    memset(&s.sh, 0, sizeof(s.sh));
    s.sh.sh_type      = SHT_REL;
    s.sh.sh_addralign = 4;
    s.sh.sh_entsize   = sizeof(Elf32_Rel);
    s.sh.sh_info      = (Elf32_Word)a->target_out_index; /* section cible dans OUT */
    /* sh_link sera fixé après quand on connaît index .symtab */

    s.data_size = (uint32_t)(a->n * sizeof(Elf32_Rel));
    s.data = (unsigned char*)malloc(s.data_size);
    if (!s.data) { free(s.name); goto done; }

    /* contenu en endian de sortie.
       Si OUT est big-endian, on swap r_offset/r_info avant de copier.
    */
    int out_big = is_big_endian_fich(ehO);
    Elf32_Rel *dst = (Elf32_Rel*)s.data;

    for (size_t k = 0; k < a->n; k++) {
      Elf32_Rel r = a->rels[k]; /* host endian */
      if (out_big) correct_endian_rel(&r);
      dst[k] = r;
    }

    s.sh.sh_size = (Elf32_Word)s.data_size;

    if (vec_push(&R, s) != 0) goto done;
  }

  /* Rebuild .shstrtab */
  {
    int shstrndx = -1;
    if (ajouter_shstrtab(&R, &shstrndx) != 0) goto done;

    /* Fixer sh_link des relocations -> index .symtab */
    int idx_symtab = vec_find_by_name(&R, ".symtab");
    if (idx_symtab < 0) {
      fprintf(stderr, "E8: impossible de trouver .symtab dans OUT\n");
      goto done;
    }

    for (int i = 1; i < R.n; i++) {
      if (R.v[i].sh.sh_type == SHT_REL) {
        R.v[i].sh.sh_link = (Elf32_Word)idx_symtab;
      }
    }

    /* Écrire out8 */
    if (ecrire_elf_resultat(fileOut8, &ehO, &R, shstrndx, is_big_endian_fich(ehO)) != 0)
      goto done;
  }

  ret = 0;

done:
  if (fa) fclose(fa);
  if (fb) fclose(fb);
  if (fo7) fclose(fo7);

  free(shstrA); free(shstrB); free(shstrO);
  if (LA) free_Shdr_list(LA);
  if (LB) free_Shdr_list(LB);
  if (LO) free_Shdr_list(LO);

  free_relaggs(aggs, naggs);
  vec_free(&R);

  return ret;
}


/**********************************ETAPE 9: il est à signaler qu'a chaque etape on construisait un bour du fichier fusion section-> correction des symboles->relocations mais  pas au meme moment
 * donc dans cette etape on va juste relire le fichier final out8.o et si erreur reconstruit shtrtab ******************************
 */

 int E9_produire_elf_final(const char *fileOut8, const char *fileOut9)
{
  int ret = -1;

  FILE *f = NULL;
  Elf32_Ehdr eh;
  Shdr_liste *L = NULL;
  char *shstr = NULL;

  VecSec R;
  vec_init(&R);

  if (charger_elf(fileOut8, &f, &eh, &L, &shstr) != 0) goto done;

  if (ajouter_section0(&R) != 0) goto done;

  /* Copier toutes les sections sauf .shstrtab (on la reconstruit pour eviter tout erreur) */
  for (int i = 1; i < (int)eh.e_shnum; i++) {
    Shdr_liste *s = section_index(L, i);
    if (!s) continue;

    const char *nm = (shstr ? shstr + s->header.sh_name : "");
    if (nm && strcmp(nm, ".shstrtab") == 0) continue;

    SecR out;
    memset(&out, 0, sizeof(out));
    out.name = dupliquer_chaine(nm);
    if (!out.name) goto done;

    out.sh = s->header;

    if (out.sh.sh_type != SHT_NOBITS && out.sh.sh_size > 0) {
      out.data_size = out.sh.sh_size;
      out.data = (unsigned char*)malloc(out.data_size);
      if (!out.data) { free(out.name); goto done; }
      memcpy(out.data, s->content, out.data_size);
    }

    if (vec_push(&R, out) != 0) goto done;
  }

  /* Rebuild .shstrtab => remet sh_name correctement */
  int shstrndx = -1;
  if (ajouter_shstrtab(&R, &shstrndx) != 0) goto done;

  int idx_symtab = vec_find_by_name(&R, ".symtab");
  int idx_strtab = vec_find_by_name(&R, ".strtab");

  /* Fix .symtab sh_link -> .strtab */
  if (idx_symtab >= 0 && idx_strtab >= 0) {
    R.v[idx_symtab].sh.sh_link = (Elf32_Word)idx_strtab;
  }

  /* Recalculer sh_info de .symtab = premier index non-local */
  if (idx_symtab >= 0) {
    TableSym T;
    memset(&T, 0, sizeof(T));

    if (charger_table_sym(f, eh, L, shstr, &T) == 0) {
      uint32_t first_global = 0;
      for (size_t i = 0; i < T.nsyms; i++) {
        if (ELF32_ST_BIND(T.syms[i].st_info) != STB_LOCAL) {
          first_global = (uint32_t)i;
          break;
        }
      }
      R.v[idx_symtab].sh.sh_info = (Elf32_Word)first_global;
      free_table_sym(&T);
    }
  }

  /* Fix sh_link des REL -> index .symtab */
  if (idx_symtab >= 0) {
    for (int i = 1; i < R.n; i++) {
      if (R.v[i].sh.sh_type == SHT_REL) {
        R.v[i].sh.sh_link = (Elf32_Word)idx_symtab;
      }
    }
  }

  /* Ecriture finale */
  if (ecrire_elf_resultat(fileOut9, &eh, &R, shstrndx, is_big_endian_fich(eh)) != 0)
    goto done;

  ret = 0;

done:
  if (f) fclose(f);
  free(shstr);
  if (L) free_Shdr_list(L);
  vec_free(&R);
  return ret;
}
