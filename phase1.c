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



const char *machine_to_string(Elf32_Ehdr h)
{
    switch (h.e_machine) {
        case EM_NONE:        return "No machine";
        case EM_M32:         return "AT&T WE 32100";
        case EM_SPARC:       return "Sun Microsystems SPARC";
        case EM_386:         return "Intel 80386";
        case EM_68K:         return "Motorola 68000";
        case EM_88K:         return "Motorola 88000";
        case EM_860:         return "Intel 80860";
        case EM_MIPS:        return "MIPS RS3000";
        case EM_PARISC:      return "HP/PA";
        case EM_SPARC32PLUS: return "SPARC (jeu d'instruction étendu)";
        case EM_PPC:         return "PowerPC";
        case EM_PPC64:       return "PowerPC 64 bits";
        case EM_S390:        return "IBM S/390";
        case EM_ARM:         return "ARM";
        case EM_SH:          return "Renesas SuperH";
        case EM_SPARCV9:     return "SPARC v9 64 bits";
        case EM_IA_64:       return "Intel Itanium";
        case EM_X86_64:      return "AMD x86-64 architecture";
        case EM_AARCH64:     return "AArch64";
        case EM_VAX:         return "DEC VAX";
        default:             return "Unknown";
    }
}


const char *type_to_string(Elf32_Ehdr header)
{
    switch (header.e_type) {
        case ET_NONE: return "NONE (No file type)";
        case ET_REL:  return "REL (Relocatable file)";
        case ET_EXEC: return "EXEC (Executable file)";
        case ET_DYN:  return "DYN (Shared object file)";
        case ET_CORE: return "CORE (Core file)";
        default:      return "Unknown";
    }
}




int E1_afficher_read_Elf32_Ehdr(Elf32_Ehdr header) {
	
	printf("=== EN-TÊTE ELF ===\n");
	// Affichage des 16 octets magiques
	printf("  Magic:   ");
	for (int i = 0; i < EI_NIDENT; i++) {        
		printf("%02x ", header.e_ident[i]);
	}
	printf("\n");
	
	// Affichage de la classe ELF (32 ou 64 bits)
	printf("  Class:                             %s\n", header.e_ident[EI_CLASS] == ELFCLASS32 ? "ELF32" : "ELF64");  // Classe ELF 1=32 et 2=64

	// Affichage de l'endianness (little ou big endian)
	printf("  Data:                              %s\n", header.e_ident[EI_DATA] == ELFDATA2MSB ?  "2's complement, big endian" :"2's complement, little endian"); // 1=little endian sinon big endian

	// Affichage de la version ELF
	printf("  Version:                           %d (current)\n", header.e_ident[EI_VERSION]); // Version ELF (1=original)

	// Affichage du OS/ABI
	const char *osabi_str;
	switch (header.e_ident[EI_OSABI]) {
		case ELFOSABI_SYSV:        osabi_str = "UNIX - System V"; break;
		case ELFOSABI_LINUX:       osabi_str = "UNIX - Linux"; break;
		default:                   osabi_str = "Unknown"; break;
	}
	printf("  OS/ABI:                            %s\n", osabi_str); // OS/ABI
	printf("  ABI Version:                       %d\n", header.e_ident[EI_ABIVERSION]); // Version ABI

	// Affichage du type de fichier
	
	printf("  Type:                              %s\n", type_to_string(header));

	// Affichage de l'architecture machine
	printf("  Machine:                           %s\n", machine_to_string(header));
	
	// Affichage de la version ELF
	printf("  Version:                           0x%x\n", header.e_version);

	// Affichage de l'adresse d'entrée et des offsets des tables
	printf("  Entry point address:               0x%x\n", header.e_entry);
	printf("  Start of program headers:          %d (bytes into file)\n", header.e_phoff);
	printf("  Start of section headers:          %d (bytes into file)\n", header.e_shoff);
	
	//Affichage des flags et tailles divers (section/program headers/etc)
	if (header.e_machine == EM_ARM) {
		int eabi = (header.e_flags >> 24) & 0xFF;
		if (eabi != 0) {
			printf("  Flags:                             0x%x, Version%u EABI\n", header.e_flags, eabi);
		} else	
		printf("  Flags:                             0x%x (ARM)\n", header.e_flags);
	} else {
	printf("  Flags:                             0x%x\n", header.e_flags);
	}
	printf("  Size of this header:               %d (bytes)\n", header.e_ehsize);
	printf("  Size of program headers:           %d (bytes)\n", header.e_phentsize);
	printf("  Number of program headers:         %d\n", header.e_phnum);	
	printf("  Size of section headers:           %d (bytes)\n", header.e_shentsize);
	printf("  Number of section headers:         %d\n", header.e_shnum);
	printf("  Section header string table index: %d\n", header.e_shstrndx);

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


void afficher_Shdr_type(Elf32_Shdr s)
{
    const char *type;

    switch (s.sh_type) {
        case SHT_NULL:     type = "NULL"; break;
        case SHT_PROGBITS: type = "PROGBITS"; break;
        case SHT_SYMTAB:   type = "SYMTAB"; break;
        case SHT_STRTAB:   type = "STRTAB"; break;
        case SHT_RELA:     type = "RELA"; break;
        case SHT_HASH:     type = "HASH"; break;
        case SHT_DYNAMIC:  type = "DYNAMIC"; break;
        case SHT_NOTE:     type = "NOTE"; break;
        case SHT_NOBITS:   type = "NOBITS"; break;
        case SHT_REL:      type = "REL"; break;
        case SHT_SHLIB:    type = "SHLIB"; break;
        case SHT_DYNSYM:   type = "DYNSYM"; break;
        default:           type = "ARM_ATTRIBUTES"; break;
    }

    printf(" %-15s", type);
}

void afficher_section_flags(Elf32_Word flags)
{
   // W = write, A = alloc, X = exec
    char buf[32];
    int pos = 0;

    if (flags & SHF_WRITE)     buf[pos++] = 'W';
    if (flags & SHF_ALLOC)     buf[pos++] = 'A';
    if (flags & SHF_EXECINSTR) buf[pos++] = 'X';
	if (flags & SHF_MERGE)            buf[pos++] = 'M';
    if (flags & SHF_STRINGS)          buf[pos++] = 'S';
    if (flags & SHF_INFO_LINK)        buf[pos++] = 'I';
    if (flags & SHF_LINK_ORDER)       buf[pos++] = 'L';
	if (flags & SHF_OS_NONCONFORMING) buf[pos++] = 'O';
    if (pos == 0) {
        buf[pos++] = ' ';
    }

    buf[pos] = '\0';

    printf("%-3s", buf);
}


void E2_afficher_Shdr_list(FILE *f, Elf32_Ehdr h, Shdr_liste *L){

 char *shstrtab = read_shstrtab(f, h);// lire la table des chaines de caracteres des noms de sections pour l'affichage des noms de sections
    if (!shstrtab) {
        fprintf(stderr, "Impossible de lire .shstrtab\n");
        return;
    }

    printf("Table des sections :\n");
    printf("  [Nr] Name               Type              Addr      Off    Size   ES Flg Lk Inf Al\n");

    Shdr_liste *p = L;
    int index = 0;

    while (p != NULL) {
        Elf32_Shdr s = p->header;
        const char *name = (s.sh_name == 0) ? "" : (shstrtab + s.sh_name);// recuperer le nom de la section a partir de sh_name et de la table des chaines de caracteres
        printf("  [%2d] %-18s", index, name);
        afficher_Shdr_type(s);  
        printf(" %08x  %06x  %06x  %02x ",
               (unsigned)s.sh_addr,
               (unsigned)s.sh_offset,
               (unsigned)s.sh_size,
               (unsigned)s.sh_entsize);
        afficher_section_flags(s.sh_flags);
        printf(" %2u  %3u  %2u" ,
               (unsigned)s.sh_link,
               (unsigned)s.sh_info,
               (unsigned)s.sh_addralign);

        printf("\n");

        p = p->next;
        index++;
    }
    free(shstrtab);
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


void E3_afficher_content_section(Shdr_liste *section)
{
    if (section == NULL) {
        fprintf(stderr, "Section inexistante.\n");
        return;
    }

    Elf32_Shdr sh = section->header;

    if (sh.sh_type == SHT_NOBITS || sh.sh_size == 0 || section->content == NULL) {
        printf("Hex dump of section: <no data>\n");
        return;
    }

    printf("Hex dump of section:\n");

    for (Elf32_Word i = 0; i < sh.sh_size; i += 16) {

        /* offset */
        printf("  0x%08x ", (unsigned)i);

        /* afficher jusqu'à 16 octets, groupés par 4 comme la commande readelf */
        for (int j = 0; j < 16; j += 4) {
            for (int k = 0; k < 4; k++) {
                Elf32_Word idx = i + j + k;
                if (idx < sh.sh_size)
                    printf("%02x", (unsigned char)section->content[idx]);
                else
                    printf("  ");
            }
            printf(" ");
        }

        printf("\n");
    }
}

//***********************************************************ETAPE 4**********************************************************************
//***************************************************************************************************************************************

// Convertit le binding d'un symbole en chaîne
const char *sym_bind_to_string(unsigned char info) {
    switch (ELF32_ST_BIND(info)) {
        case STB_LOCAL:  return "LOCAL";
        case STB_GLOBAL: return "GLOBAL";
        case STB_WEAK:   return "WEAK";
        default:         return "UNKNOWN";
    }
}

// Convertit le type d'un symbole en chaîne
const char *sym_type_to_string(unsigned char info) {
    switch (ELF32_ST_TYPE(info)) {
        case STT_NOTYPE:  return "NOTYPE";
        case STT_OBJECT:  return "OBJECT";
        case STT_FUNC:    return "FUNC";
        case STT_SECTION: return "SECTION";
        case STT_FILE:    return "FILE";
        default:          return "UNKNOWN";
    }
}

// Convertit la visibilité d'un symbole en chaîne
const char *sym_visibility_to_string(unsigned char info) {
    switch (info & 0x3) {  // Masque ELF32_ST_VISIBILITY
        case STV_DEFAULT:   return "DEFAULT";
        case STV_INTERNAL:  return "INTERNAL";
        case STV_HIDDEN:    return "HIDDEN";
        case STV_PROTECTED: return "PROTECTED";
        default:            return "UNKNOWN";
    }
}

// Correction de l'endian pour un symbole ELF32
void correct_endian_sym(Elf32_Sym *s) {
    s->st_name  = reverse_4(s->st_name);
    s->st_value = reverse_4(s->st_value);
    s->st_size  = reverse_4(s->st_size);
    s->st_shndx = reverse_2(s->st_shndx);
}

void E4_afficher_symtab(FILE *f, Elf32_Ehdr h, Shdr_liste *L) {

    Shdr_liste *symtab = NULL;

    // 1. trouver la section SYMTAB
    for (Shdr_liste *p = L; p != NULL; p = p->next) {
        if (p->header.sh_type == SHT_SYMTAB) {
            symtab = p;
            break;
        }
    }

    if (!symtab) {
        printf("Aucune table des symboles trouvée.\n");
        return;
    }

    // 2. retrouver la table de chaînes associée
    Shdr_liste *strtab = section_index(L, symtab->header.sh_link);
    if (!strtab) {
        fprintf(stderr, "Erreur: .strtab introuvable\n");
        return;
    }
    /* 3) Lire la table des noms de sections (.shstrtab) pour STT_SECTION */
    char *shstrtab = read_shstrtab(f, h);
    if (!shstrtab) {
        fprintf(stderr, "Impossible de lire .shstrtab\n");
        return;
    }

    Elf32_Sym *symbols = (Elf32_Sym *)symtab->content;
    int nb = symtab->header.sh_size / sizeof(Elf32_Sym);

    printf("\nSymbol table '.symtab' contains %d entries:\n", nb);
    printf("   Num:    Value  Size Type     Bind     Vis      Ndx Name\n");

    for (int i = 0; i < nb; i++) {
        Elf32_Sym s = symbols[i];

        if (is_big_endian_fich(h))
            correct_endian_sym(&s);
        /* Name */
        const char *name = "";

        if (ELF32_ST_TYPE(s.st_info) == STT_SECTION) {// symbole de type section
            /* Nom = nom de la section st_shndx (dans .shstrtab) */
            Shdr_liste *sec = section_index(L, s.st_shndx);
            if (sec && sec->header.sh_name != 0) {
                name = shstrtab + sec->header.sh_name;
            }
        } else {
            /* Nom normal = dans .strtab via st_name */
            if (s.st_name != 0) {
                name = (const char *)((char *)strtab->content + s.st_name);
            }
        }

        // Calcul de Ndx (index de section ou spécial)
        char ndx[8];
        if (s.st_shndx == SHN_UNDEF)       sprintf(ndx, "UND");
        else if (s.st_shndx == SHN_ABS)    sprintf(ndx, "ABS");
        else if (s.st_shndx == SHN_COMMON) sprintf(ndx, "COM");
        else                               sprintf(ndx, "%3u", s.st_shndx);

        printf("%6d: %08x %5u %-8s %-8s %-8s %s %s\n",
               i,
               (unsigned)s.st_value,
               (unsigned)s.st_size,
               sym_type_to_string(s.st_info),
               sym_bind_to_string(s.st_info),
               sym_visibility_to_string(s.st_other),
               ndx,
               name);
    }
    free(shstrtab);
}

//***********************************************************ETAPE 5**********************************************************************
//***************************************************************************************************************************************

const char *arm_rel_type(unsigned type)
{
    switch (type) {
        case R_ARM_NONE:   return "R_ARM_NONE";
        case R_ARM_ABS32:  return "R_ARM_ABS32";
        case R_ARM_REL32:  return "R_ARM_REL32";
        case R_ARM_CALL:   return "R_ARM_CALL";
        case R_ARM_JUMP24: return "R_ARM_JUMP24";
        case R_ARM_V4BX: return "R_ARM_V4BX";
        default: return "R_ARM_<UNKNOWN>";
    }
}


void E5_afficher_relocation(FILE *f, Elf32_Ehdr h, Shdr_liste *L)
{
    /* table des noms de sections */
    char *shstrtab = read_shstrtab(f, h);
    if (!shstrtab) {
        fprintf(stderr, "Erreur: impossible de lire .shstrtab\n");
        return;
    }

    for (Shdr_liste *relsec = L; relsec != NULL; relsec = relsec->next) {

        Elf32_Shdr sh = relsec->header;
        if (sh.sh_type != SHT_REL && sh.sh_type != SHT_RELA)
            continue;

        /* section cible patchée + symtab associée */
        Shdr_liste *target = section_index(L, sh.sh_info); // index de la section qu'on va modifier
        Shdr_liste *symtab = section_index(L, sh.sh_link); // index de la table de symboles
        if (!target || !symtab) {
            fprintf(stderr,
                    "Erreur: section cible ou symtab introuvable pour la section de relocation\n");
            continue;
        }

        /* nom de la section de relocation  */
        const char *relname = (sh.sh_name == 0) ? "" : (shstrtab + sh.sh_name);

        /* nombre d'entrées */
        int n = (sh.sh_type == SHT_REL)
                    ? sh.sh_size / sizeof(Elf32_Rel)
                    : sh.sh_size / sizeof(Elf32_Rela);

        
        printf("\nRelocation section '%s' at offset 0x%x contains %d entries:\n",
               relname,
               (unsigned)sh.sh_offset,
               n);

        if (sh.sh_type == SHT_REL) {

            Elf32_Rel *rels = (Elf32_Rel *)relsec->content;
            printf("  Offset     Type              Sym\n");

            for (int i = 0; i < n; i++) {
                Elf32_Rel r = rels[i];

                if (is_big_endian_fich(h)) {
                    r.r_offset = reverse_4(r.r_offset); //ou on va appliquer la relocation dans la section cible
                    r.r_info   = reverse_4(r.r_info);   //type de relocation + symbole concerne
                }
                unsigned sym  = ELF32_R_SYM(r.r_info);  //on extrait l'index du symbole dans symtab
                unsigned type = ELF32_R_TYPE(r.r_info);  //on extrait le type de relocation
                

                printf("  %08x  %-16s  %u\n",
                       (unsigned)r.r_offset,
                       arm_rel_type(type),
                       sym);
            }

        } else { /* SHT_RELA */

            Elf32_Rela *rels = (Elf32_Rela *)relsec->content;
            printf("  Offset     Type              Sym   Addend\n");

            for (int i = 0; i < n; i++) {
                Elf32_Rela r = rels[i];

                if (is_big_endian_fich(h)) {
                    r.r_offset = reverse_4(r.r_offset);
                    r.r_info   = reverse_4(r.r_info);
                    r.r_addend = reverse_4(r.r_addend);
                }

                unsigned sym  = ELF32_R_SYM(r.r_info);
                unsigned type = ELF32_R_TYPE(r.r_info);

                printf("  %08x  %-16s  %u   %d\n",
                       (unsigned)r.r_offset,
                       arm_rel_type(type),
                       sym,
                       (int)r.r_addend);
            }
        }
    }

    free(shstrtab);
}

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


/************************************************************************************************************ */
/************************************************************************************************************ */
/************************************************************************************************************ */
/************************************************************************************************************ */
/************************************************************************************************************ */
/************************************************************************************************************ */

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

/* Ajoute une section au vector. */
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
