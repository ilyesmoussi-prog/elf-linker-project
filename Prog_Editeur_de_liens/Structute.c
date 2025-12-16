#include "Structure.h"




int is_big_endian(Elf32_Ehdr h) {
    return (h.e_ident[ENDIANESS_INDEX] == ELFDATA2MSB);
}


int32_t recuperer_valeur32(Elf32_Ehdr h, int32_t value) {
    if (is_big_endian(h)) {
		printf("BSWAP 32\n");
        return (int32_t)bswap_32((uint32_t)value);
    }
    return value;
}


int16_t recuperer_valeur16(Elf32_Ehdr h, int32_t value) {
    uint16_t v16 = (uint16_t)value;
    if (is_big_endian(h)) {
		printf("BSWAP 16\n");
        v16 = bswap_16(v16);
    }
    return (int16_t)v16;
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


    int is_bigend = is_big_endian(*h);

    // Si le fichier est en big-endian, il faut inverser les octets des champs
    if (is_bigend) {
		h->e_type = recuperer_valeur16(*h, h->e_type);
		h->e_machine = recuperer_valeur16(*h, h->e_machine);
		h->e_version = recuperer_valeur32(*h, h->e_version);
		h->e_entry = recuperer_valeur32(*h, h->e_entry);
		h->e_phoff = recuperer_valeur32(*h, h->e_phoff);
		h->e_shoff = recuperer_valeur32(*h, h->e_shoff);//debut des en tete de section 
		h->e_flags = recuperer_valeur32(*h, h->e_flags);
		h->e_ehsize = recuperer_valeur16(*h, h->e_ehsize);
		h->e_phentsize = recuperer_valeur16(*h, h->e_phentsize);
		h->e_phnum = recuperer_valeur16(*h, h->e_phnum);
		h->e_shentsize = recuperer_valeur16(*h, h->e_shentsize); //taille d'une entree en tete de section
		h->e_shnum = recuperer_valeur16(*h, h->e_shnum); //nb entree en tete de section
		h->e_shstrndx = recuperer_valeur16(*h, h->e_shstrndx); //nom des sections 
    }

    return 0;
}

void afficher_Elf32_Ehdr( Elf32_Ehdr h )
{
    printf(
        "HEADER  : \n"
        "\te_type : %d\n"
        "\te_machine : %d\n"
        "\te_version : %d\n"
        "\te_entry : 0x%x\n"
        "\te_phoff : 0x%x\n"
        "\te_shoff : 0x%x\n"
        "\te_flags : %d\n"
        "\te_ehsize : %d\n"
        "\te_phentsize : %d\n"
        "\te_phnum : %d\n"
        "\te_shentsize : %d\n"
        "\te_shnum : %d\n"
        "\te_shstrndx : %d\n\n",
        h.e_type, h.e_machine, h.e_version, h.e_entry, h.e_phoff,
        h.e_shoff, h.e_flags, h.e_ehsize, h.e_phentsize, h.e_phnum,
        h.e_shentsize, h.e_shnum, h.e_shstrndx
    );	
}
void afficher_indent(Elf32_Ehdr h){
	int i;
	printf(" Magique:    ");
	for(i=0;i<16;i++){
		if(h.e_ident[i]/16 == 0){
			printf(" 0%x", h.e_ident[i]);
		}else{
			printf(" %02x",h.e_ident[i]);
		}
	}
	printf("\n");
}
void afficher_version(Elf32_Ehdr h){
	switch(h.e_version){
		case 0 :
			printf(" Version:\t\t\t\t%d (Invalid Version)\n",h.e_version);
			break;
		case 1 :
			printf(" Version:\t\t\t\t%d (Current Version)\n",h.e_version);
			break;
	}
}
void afficher_type(Elf32_Ehdr h){
	switch(h.e_type){
		case ET_NONE :
			printf(" Type:\t\t\t\t\tNo file type\n");
			break;
		case ET_REL :
			printf(" Type:\t\t\t\t\tRelocatable file\n");
			break;
		case 2 :
			printf(" Type:\t\t\t\t\tExecutable file\n");
			break;
		case 3 :
			printf(" Type:\t\t\t\t\tShared object file\n");
			break;
		case 4 :
			printf(" Type:\t\t\t\t\tCore file\n");
			break;
		default :
			printf(" ERROR ON THE TYPE OF THE FILE ;)\n");
			break;
	}
}

void afficher_machine(Elf32_Ehdr h){
	switch(h.e_machine){
		case EM_NONE :
			printf(" Machine:\t\t\t\tmachine inconne");
			break;
		case EM_M32 :
			printf(" Machine:\t\t\t\tAT&T WE 32100");
			break;
		case EM_SPARC :
			printf(" Machine:\t\t\t\tSun Microsystems SPARC");
			break;
		case EM_386 :
			printf(" Machine:\t\t\t\tIntel 80386");
			break;
		case EM_68K :
			printf(" Machine:\t\t\t\tMotorola 68000");
			break;
		case EM_88K :
			printf(" Machine:\t\t\t\tMotorola 88000");
			break;
		case EM_860 :
			printf(" Machine:\t\t\t\tIntel 80860");
			break;
		case EM_MIPS :
			printf(" Machine:\t\t\t\tMIPS RS3000");
			break;
		case EM_PARISC :
			printf(" Machine:\t\t\t\tHP/PA");
			break;
		case EM_SPARC32PLUS :
			printf(" Machine:\t\t\t\tSPARC avec jeu d'instruction étendu");
			break;
		case EM_PPC :
			printf(" Machine:\t\t\t\tPowerPC");
			break;
		case EM_PPC64 :
			printf(" Machine:\t\t\t\tPowerPC 64 bits");
			break;
		case EM_S390 :
			printf(" Machine:\t\t\t\tIBM S/390");
			break;
		case EM_ARM :
			printf(" Machine:\t\t\t\tARM ");
			break;
		case EM_SH :
			printf(" Machine:\t\t\t\tRenesas SuperH");
			break;
		case EM_SPARCV9 :
			printf(" Machine:\t\t\t\tSPARCC v9 64 bits");
			break;
		case EM_IA_64 :
			printf(" Machine:\t\t\t\tIntel Itanium");
			break;
		case EM_X86_64 :
			printf(" Machine:\t\t\t\tAMD x86-64");
			break;
		case EM_VAX :
			printf(" Machine:\t\t\t\tDEC Vax");
			break;
		default :
			printf(" ERROR ON THE FILE'S 'MACHINE'");
			break;
	}
	printf("\n");
}

void afficher_en_tete_format_commande(Elf32_Ehdr h){
	printf("En_tête ELF:\n");
	afficher_indent(h);
	printf(" Classe:\t\t\t\tELF32\n");	
	if(is_big_endian(h)){
		printf(" Donnees:\t\t\t\t2's complement, big endian\n");
	}
	else{
		printf(" Donnees:\t\t\t\tlittle endian\n");
	}

	afficher_type(h);
	afficher_machine(h);
	afficher_version(h);

	printf(" Addresse du point d'entrée:\t\t0x%x\n",h.e_entry);
	printf(" Début des en_têtes du programme:\t%d\n", h.e_phoff);
	printf(" Shoff:\t\t\t\t\t%d\n",h.e_shoff);
	printf(" Flags:\t\t\t\t\t0x%x\n",h.e_flags);
	printf(" Taille de l'en-tête (en octets):\t%d\n", h.e_ehsize);
	printf(" Taille d'une entrée de la table d'en tête (en octets):\t%d\n", h.e_phentsize);
	printf(" Nombre d'entrée de la table d'en tête:\t%d\n", h.e_phnum);
	printf(" Taille de section headers:\t\t%d (bytes)\n",h.e_shentsize);
	printf(" Nombre de section headers:\t\t%d\n",h.e_shnum);
	printf(" L'index de la table des section headers:\t%d\n",h.e_shstrndx);

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

    if (is_big_endian(h)) {
        s->sh_name      = recuperer_valeur32(h, s->sh_name);
        s->sh_type      = recuperer_valeur32(h, s->sh_type);
        s->sh_flags     = recuperer_valeur32(h, s->sh_flags);
        s->sh_addr      = recuperer_valeur32(h, s->sh_addr);
        s->sh_offset    = recuperer_valeur32(h, s->sh_offset);
        s->sh_size      = recuperer_valeur32(h, s->sh_size);
        s->sh_link      = recuperer_valeur32(h, s->sh_link);
        s->sh_info      = recuperer_valeur32(h, s->sh_info);
        s->sh_addralign = recuperer_valeur32(h, s->sh_addralign);
        s->sh_entsize   = recuperer_valeur32(h, s->sh_entsize);
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
        buf[pos++] = '-';
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
}
