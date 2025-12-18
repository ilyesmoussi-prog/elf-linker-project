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
