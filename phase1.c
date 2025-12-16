#include "phase1.h"
#include "util.h"



int is_big_endian_fich(Elf32_Ehdr h) {
    return (h.e_ident[ENDIANESS_INDEX] == ELFDATA2MSB);
}


int read_Elf32_Ehdr(FILE *f, Elf32_Ehdr *h)
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




int afficher_read_Elf32_Ehdr(Elf32_Ehdr header) {
	
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

//################################################################################################################################
//################################################################################################################################
//lire le header de section 
int read_Elf32_Shdr(FILE *f, Elf32_Ehdr h, unsigned int index, Elf32_Shdr *s)
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

char *read_shstrtab(FILE *f, Elf32_Ehdr h)
{
    Elf32_Shdr shstr_hdr;

    if (read_Elf32_Shdr(f, h, h.e_shstrndx, &shstr_hdr) != 0) {
        fprintf(stderr, "Impossible de lire le header de .shstrtab\n");
        return NULL;
    }

    char *shstrtab = malloc(shstr_hdr.sh_size);
    if (!shstrtab) {
        perror("allocation shstrtab");
        return NULL;
    }

    if (fseek(f, shstr_hdr.sh_offset, SEEK_SET) != 0) {
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
void read_Shdr_list(FILE *f, Elf32_Ehdr h, Shdr_liste * L)
{
    int i;

    Shdr_liste * p = L, *N;
    L->next = NULL;
    
    read_Elf32_Shdr(f, h, 0, &(L->header));
    for( i = 1; i < h.e_shnum; i++ )
    {
        N = malloc( sizeof(Shdr_liste) );
        read_Elf32_Shdr(f, h, i, &(N->header));
        fseek( f, N->header.sh_offset, SEEK_SET );
        N->content = malloc( N->header.sh_size );
        fread( N->content, N->header.sh_size, 1, f); 
        //section suivante
        p->next = N;
        p = N;
        N->next = NULL;
    }
}


void afficher_Shdr_type(Elf32_Shdr s){
	switch(s.sh_type){
		case SHT_NULL :
			printf(" %15s ","NULL");
			break;
		case SHT_PROGBITS :
			printf(" %15s ","PROGBITS");
			break;
		case SHT_SYMTAB :
			printf(" %15s ","SYMTAB");
			break;
		case SHT_STRTAB :
			printf(" %15s ","STRTAB");
			break;
		case SHT_RELA :
			printf(" %15s ","RELA");
			break;
		case SHT_HASH :
			printf(" %15s ","HASH");
			break;
		case SHT_DYNAMIC :
			printf(" %15s ","DYNAMIC");
			break;
		case SHT_NOTE :
			printf(" %15s ","NOTE");
			break;
		case SHT_NOBITS :
			printf(" %15s ","NOBITS");
			break;
		case SHT_REL :
			printf(" %15s ","REL");
			break;
		case SHT_SHLIB :
			printf(" %15s ","SHLIB");
			break;
		case SHT_DYNSYM :
			printf(" %15s ","DYNSYM");
			break;
		case SHT_LOPROC :
			printf(" %15s ","LOPROC");
			break;
		default :
			printf(" %15s ","ARM_ATTRIBUTES");
			break;
	}
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


void afficher_Shdr_list(FILE *f, Elf32_Ehdr h, Shdr_liste *L){

    char *shstrtab = read_shstrtab(f, h);
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
        const char *name = (s.sh_name == 0) ? "" : (shstrtab + s.sh_name);
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

}
/*
static Shdr_liste* section_index(Shdr_liste *L, int idx) {
    int i = 0;
    for (Shdr_liste *p = L; p != NULL; p = p->next, i++) {
        if (i == idx) return p;
    }
    return NULL;
}

static Shdr_liste* section_name(FILE *f, Elf32_Ehdr h, Shdr_liste *L, const char *target) {
    char *shstrtab = read_shstrtab(f, h);
    if (!shstrtab) return NULL;

    for (Shdr_liste *p = L; p != NULL; p = p->next) {
        Elf32_Shdr s = p->header;
        const char *name = (s.sh_name == 0) ? "" : (shstrtab + s.sh_name);
        if (strcmp(name, target) == 0) {
            return p;
        }
    }

    return NULL;
}*/




/*

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <fichier-elf>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    Elf32_Ehdr hdr;
    if (read_Elf32_Ehdr(f, &hdr) != 0) {
        fprintf(stderr, "Erreur: impossible de lire l'en-tete ELF.\n");
        fclose(f);
        return EXIT_FAILURE;
    }

    //afficher_Elf32_Ehdr(hdr);
	//afficher_en_tete_format_commande(hdr);
	// Construire la liste des sections
    Shdr_liste head;
    read_Shdr_list(f, hdr, &head);

     //Afficher la table des sections (Etape 2)
     afficher_Shdr_list(f, hdr, &head);

    fclose(f);
    return EXIT_SUCCESS;
}*/
