/*
ELF Loader - chargeur/implanteur d'exécutables au format ELF à but pédagogique
Copyright (C) 2012 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique Générale GNU publiée par la fee Software
Foundation (version 2 ou bien toute autre version ultérieure choisie par vous).

Ce programme est distribué car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but spécifique. Reportez-vous à la
Licence Publique Générale GNU pour plus de détails.

Vous devez avoir reçu une copie de la Licence Publique Générale GNU en même
temps que ce programme ; si ce n'est pas le cas, écrivez à la fee Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
États-Unis.

Contact: Guillaume.Huard@imag.f
         ENSIMAG - Laboratoire LIG
         51 avenue Jean Kuntzmann
         38330 Montbonnot Saint-Martin
*/
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "phase1.h"

void usage(char *name) {
	fprintf(stderr, "Usage:\n"
		"%s\n [ --debug file ] : mode debug\n"
		" [--help] : affiche cette aide\n"
		" [ -t file ] : Affiche l'entete ELF\n"
		" [ -S file ] : Affiche la table des sections ELF\n"
		" [ -x file ] : Affiche le contenu de la section ELF\n"
		" [ -s file ] : Affiche la table des symboles ELF\n"
		" [ -r file ] : Affiche la table de relocation ELF\n",
		name);
}


int main(int argc, char *argv[]) {
	int opt;

	struct option longopts[] = {
		{ "debug", required_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};

	while ((opt = getopt_long(argc, argv, "d:h:t:S:x:s:r:", longopts, NULL)) != -1) {
		const char *filename = NULL;
		if (opt == 'x') {
			if (optind >= argc) {
				fprintf(stderr, "Erreur: fichier manquant pour -x %s\n\n", optarg);
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			filename = argv[optind];   // fichier ELF après ".text" ou "1"
		} else {
			filename = optarg;         // fichier ELF directement dans optarg
		}
		FILE *f = fopen(filename, "rb");
		if (f == NULL) {
			perror("fopen");
			exit(EXIT_FAILURE);
		}
		Elf32_Ehdr hdr;
		if (E1_read_Elf32_Ehdr(f, &hdr) != 0) {
			fprintf(stderr, "Erreur: impossible de lire l'en-tete ELF.\n");
			fclose(f);
			return EXIT_FAILURE;
		}

		Shdr_liste *L = malloc(sizeof(Shdr_liste));
		if (L == NULL) {
			perror("malloc");
			fclose(f);
			return EXIT_FAILURE;
		}
		switch(opt) {

		case 't':

			E1_afficher_read_Elf32_Ehdr(hdr);
			fclose(f);
			break;
		case 'S':	
			E2_read_Shdr_list(f, hdr, L);
			E2_afficher_Shdr_list(f, hdr, L);
			fclose(f);
			break;
		case 's': {

			E2_read_Shdr_list(f, hdr, L);
			E4_afficher_symtab(f, hdr, L);

			fclose(f);
			break;
		}	
		case 'x': {
			const char *secname = optarg;  

			E2_read_Shdr_list(f, hdr, L);
			Shdr_liste *section = NULL;
			char *endptr;
			long idx = strtol(secname, &endptr, 10);

			if (*endptr == '\0') {
				section = section_index(L, (int)idx);
			} else {
				section = section_name(f, hdr, L, secname);
			}
			if (section == NULL) {
				fprintf(stderr, "Section .text introuvable\n");
				fclose(f);
				exit(EXIT_FAILURE);
			}

			E3_afficher_content_section(section);

			fclose(f);

			break;
		}
		case 'r': {

			E2_read_Shdr_list(f, hdr, L);
			E5_afficher_relocation(f, hdr, L);

			fclose(f);
			break;
		}

		case 'h':
			usage(argv[0]);
			exit(0);
		case 'd':
			add_debug_to(optarg);
			break;
		default:
			fprintf(stderr, "Erreur: aucune action Valide\n\n");
			usage(argv[0]);
			exit(1);
		}
		free_Shdr_list(L);
	}

	return 0;
}