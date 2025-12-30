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
		" [ -r file ] : Affiche la table de relocation ELF\n"
		" [ -f fileA fileB out ] : Etape 6 - fusionner sections (A puis B)\n",
		name);
}


int main(int argc, char *argv[]) {
	int opt;

	struct option longopts[] = {
		{ "debug", required_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};

	while ((opt = getopt_long(argc, argv, "d:h:t:S:x:s:r:f:", longopts, NULL)) != -1) {
		const char *filename = NULL;
		if (opt == 'x') {
			if (optind >= argc) {
				fprintf(stderr, "Erreur: fichier manquant pour -x %s\n\n", optarg);
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			filename = argv[optind];   // fichier ELF après ".text" ou "1"
		}else if (opt == 'f') {
			if (optind + 1 >= argc) {
				fprintf(stderr, "Erreur: usage -f fileA fileB out\n\n");
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}
      filename = optarg;         // on ouvre fileA (comme les autres options)
		} 
		else {
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
		case 'f': {
			const char *fileA = optarg;
			const char *fileB = argv[optind];
			const char *out   = argv[optind + 1];

			/* On ferme le fichier déjà ouvert (fileA) car E6 ouvrira ses propres fichiers */
			if (f) fclose(f);
			f = NULL;

			uint32_t *renumB = NULL;
			uint32_t *deltaB = NULL;
			uint32_t *renumA = NULL;
			size_t lenA = 0;
			size_t lenB = 0;

			int rc = E6_fusionner_sections(fileA, fileB, out, &renumA, &lenA, &renumB, &deltaB, &lenB);
			if (rc != 0) {
			fprintf(stderr, "Erreur: fusion etape 6 a echoue.\n");
			free(renumB);
			free(deltaB);
			free(renumA);
			free_Shdr_list(L);
			exit(EXIT_FAILURE);
			}

			printf("Fusion OK: %s + %s -> %s\n", fileA, fileB, out);

			/* Optionnel pour debug : afficher quelques infos */
			// for (size_t i = 0; i < lenB; i++) {
			//   if (renumB[i] != 0 || deltaB[i] != 0)
			//     printf("B: sec %zu -> R: %u, delta=%u\n", i, renumB[i], deltaB[i]);
			// }

			free(renumB);
			free(deltaB);
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