#include "phase1.h"
#include "util.h"



int is_big_endian_fich(Elf32_Ehdr h) {
    return (h.e_ident[ENDIANESS_INDEX] == ELFDATA2MSB);
}
/************************************ETAPE 1 ***************************************************/
/********************************************************************************************* */
//recuperer l'entete ELF du fichier dans la structure h
int E1_read_Elf32_Ehdr(FILE *f, Elf32_Ehdr *h)
{
    if (fseek(f, 0, SEEK_SET) != 0) {
        perror("fseek");
        return -1;
    }

   
    if (fread(h, 1, sizeof(Elf32_Ehdr), f) != sizeof(Elf32_Ehdr)) {
        fprintf(stderr, "Erreur lors de la lecture de l'en-tete ELF\n");
        return -1;
    }

    // verif de la magie ELF
    if (h->e_ident[EI_MAG0] != 0x7f ||
        h->e_ident[EI_MAG1] != 'E'  ||
        h->e_ident[EI_MAG2] != 'L'  ||
        h->e_ident[EI_MAG3] != 'F') {

        fprintf(stderr, "Erreur: ce fichier n'est pas un ELF.\n");
        return -1;
    }

    int is_bigend = is_big_endian_fich(*h);

    // Si le fichier est en big-endian, il faut inverser les octets des champs
    //reverse 2 ou 4 octets selon le type du champs
    if (is_bigend) {
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

    return 0;
}


//***********************************************************ETAPE 2**********************************************************************
//*************************************************************************************************************************************** */
//lire le header de section 
int E2_read_Elf32_Shdr(FILE *f, Elf32_Ehdr h, unsigned int index, Elf32_Shdr *s)
{
    Elf32_Off offset = h.e_shoff + index * h.e_shentsize;

    if (fseek(f, offset, SEEK_SET) != 0) {
        perror("fseek section");
        return -1;
    }

    if (fread(s, 1, sizeof(Elf32_Shdr), f) != sizeof(Elf32_Shdr)) {
        fprintf(stderr, "Erreur lors de la lecture d'un Elf32_Shdr\n");
        return -1;
    }

    if (is_big_endian_fich(h)) {
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

    return 0;
}
//lire la table des chaines de caracteres des noms de sections
char *read_shstrtab(FILE *f, Elf32_Ehdr h)
{
    Elf32_Shdr shstr_hdr;

    if (E2_read_Elf32_Shdr(f, h, h.e_shstrndx, &shstr_hdr) != 0) {
        fprintf(stderr, "Impossible de lire le header de .shstrtab\n");
        return NULL;
    }

    char *shstrtab = malloc(shstr_hdr.sh_size);
    if (!shstrtab) {
        perror("allocation shstrtab");
        return NULL;
    }

    if (fseek(f, shstr_hdr.sh_offset, SEEK_SET) != 0) {//se placer  au debut de la section shstrtab
        perror("fseek shstrtab");
        free(shstrtab);
        return NULL;
    }

    if (fread(shstrtab, 1, shstr_hdr.sh_size, f) != shstr_hdr.sh_size) {
        fprintf(stderr, "Erreur lors de la lecture .shstrtab\n");
        free(shstrtab);
        return NULL;
    }

    return shstrtab;
}



//lecture de la liste des sections
void E2_read_Shdr_list(FILE *f, Elf32_Ehdr h, Shdr_liste * L)
{
    int i;

    Shdr_liste * p = L, *N;
    L->next = NULL;
    
    E2_read_Elf32_Shdr(f, h, 0, &(L->header));// lire la premiere section (index 0)
    for( i = 1; i < h.e_shnum; i++ )
    {
        N = malloc( sizeof(Shdr_liste) );
        E2_read_Elf32_Shdr(f, h, i, &(N->header));
        //section suivante
        if (fseek(f, N->header.sh_offset, SEEK_SET) != 0) {
            perror("fseek section content");
            
        }
        N->content = malloc(N->header.sh_size);
        if (!N->content) {
            perror("malloc section content");
        }
        size_t r = fread(N->content, 1, N->header.sh_size, f);// lire le contenu de la section
        if (r != N->header.sh_size) {
            fprintf(stderr, "Erreur fread: lu %zu octets sur %u\n", r, (unsigned)N->header.sh_size);
            
        }

        p->next = N;
        p = N;
        N->next = NULL;
    }
}



//***********************************************************ETAPE 3**********************************************************************
//***************************************************************************************************************************************
//lire le contenu d'une section par son index ou son nom
 Shdr_liste* section_index(Shdr_liste *L, int idx) {
    int i = 0;
    for (Shdr_liste *p = L; p != NULL; p = p->next, i++) {
        if (i == idx) return p;
    }
    return NULL;
}

Shdr_liste* section_name(FILE *f, Elf32_Ehdr h, Shdr_liste *L, const char *target) {
    char *shstrtab = read_shstrtab(f, h);
    if (!shstrtab) return NULL;

    for (Shdr_liste *p = L; p != NULL; p = p->next) {
        Elf32_Shdr s = p->header;
        const char *name = (s.sh_name == 0) ? "" : (shstrtab + s.sh_name);
        if (strcmp(name, target) == 0) {
            free(shstrtab);
            return p;
        }
    }

    free(shstrtab);
    return NULL;
}



//***********************ETAPE 4/ETAPE 5 : Affichage ***************************************************/


/********************************************************************************************************** */
/**********************************************************Free************************************************** */
void free_Shdr_list(Shdr_liste *L)
{
    Shdr_liste *p = L;
    while (p) {
        Shdr_liste *next = p->next;

        /* libérer le contenu de la section */
        if (p->content) {
            free(p->content);
        }

        
        free(p);

        p = next;
    }
}









