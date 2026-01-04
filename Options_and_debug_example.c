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
#include "phase2.h"

void usage(char *name) {
	fprintf(stderr, "Usage:\n"
		"%s\n [ --debug file ] : mode debug\n"
		" [--help] : affiche cette aide\n"
		" [ -t file ] : Affiche l'entete ELF\n"
		" [ -S file ] : Affiche la table des sections ELF\n"
		" [ -x file ] : Affiche le contenu de la section ELF\n"
		" [ -s file ] : Affiche la table des symboles ELF\n"
		" [ -r file ] : Affiche la table de relocation ELF\n"
		" [ -f fileA fileB out ] : Etape 6 - fusionner sections (A puis B)\n"
		" [ -g fileA fileB out6 out7 ] : Etape 7 - fusionner/corriger symboles (apres etape 6)\n"
		" [ -w fileA fileB out ] : Etape 8 - fusionner/corriger symboles et relocations (apres etape 6)\n"
		" [ -z out8 out9 ] : Etape 9 - finaliser/normaliser ELF (shstrtab + sh_link/sh_info) \n",
		name);
}


int main(int argc, char *argv[]) {
	int opt;

	struct option longopts[] = {
		{ "debug", required_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};

	while ((opt = getopt_long(argc, argv, "d:h:t:S:x:s:r:f:g:w:z:", longopts, NULL)) != -1) {
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
		}else if (opt == 'g') {
			if (optind + 2 >= argc) {
				fprintf(stderr, "Erreur: usage -g fileA fileB out6 out7\n\n");
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			filename = optarg; 
		} else if (opt == 'z') {
			if (optind >= argc) {
				fprintf(stderr, "Erreur: usage -z out8 out9\n\n");
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}
      filename = optarg;  // out8
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

			free(renumB);
			free(deltaB);
			break;
      }
		case 'g': {
			const char *fileA = optarg;
			const char *fileB = argv[optind];
			const char *out6  = argv[optind + 1];
			const char *out7  = argv[optind + 2];

			/* fermer le fichier déjà ouvert (fileA) */
			if (f) fclose(f);
			f = NULL;

			uint32_t *renumA = NULL, *renumB = NULL, *deltaB = NULL;
			size_t lenA = 0, lenB = 0;
			if (E6_fusionner_sections(fileA, fileB, out6, &renumA, &lenA, &renumB, &deltaB, &lenB) != 0) {
				fprintf(stderr, "Erreur: etape 6 a echoue.\n");
				free(renumA); free(renumB); free(deltaB);
				free_Shdr_list(L);
				exit(EXIT_FAILURE);
			}
			printf("E6 OK: %s + %s -> %s\n", fileA, fileB, out6);

			uint32_t *renumSymA = NULL, *renumSymB = NULL;
			size_t nSymA = 0, nSymB = 0;

			if (E7_fusionner_corriger_symboles(fileA, fileB, out6, out7,
												renumA, lenA,
												renumB, deltaB, lenB,
												&renumSymA, &nSymA,
												&renumSymB, &nSymB) != 0) {
				fprintf(stderr, "Erreur: etape 7 a echoue.\n");
				free(renumA); free(renumB); free(deltaB);
				free(renumSymA); free(renumSymB);
				free_Shdr_list(L);
				exit(EXIT_FAILURE);
			}

			printf("E7 OK: %s -> %s (symA=%zu symB=%zu)\n", out6, out7, nSymA, nSymB);

			/* libérations */
			free(renumA);
			free(renumB);
			free(deltaB);
			free(renumSymA);
			free(renumSymB);

			break;
			}
		case 'w': {
			// Usage: ./Options_and_debug_example -w fileA.o fileB.o out8.o

			if (optind + 2 > argc) {
				fprintf(stderr, "Usage: %s -w fileA.o fileB.o out8.o\n", argv[0]);
				exit(EXIT_FAILURE);
			}

			const char *fileA  = optarg;
			const char *fileB = argv[optind];
			const char *out8 = argv[optind + 1];

			const char *out6 = "/tmp/out6.o";
			const char *out7 = "/tmp/out7.o";

			uint32_t *renumA = NULL, *renumB = NULL, *deltaB = NULL;
			size_t lenA = 0, lenB = 0;

			uint32_t *renumSymA = NULL, *renumSymB = NULL;
			size_t nSymA = 0, nSymB = 0;

			// ---- E6
			if (E6_fusionner_sections(fileA, fileB, out6,
									&renumA, &lenA,
									&renumB, &deltaB, &lenB) != 0) {
				fprintf(stderr, "[-w] Erreur E6\n");
				goto w_fail;
			}

			// ---- E7
			if (E7_fusionner_corriger_symboles(fileA, fileB,
											out6, out7,
											renumA, lenA,
											renumB, deltaB, lenB,
											&renumSymA, &nSymA,
											&renumSymB, &nSymB) != 0) {
				fprintf(stderr, "[-w] Erreur E7\n");
				goto w_fail;
			}

			// ---- E8
			if (E8_fusionner_corriger_relocations(fileA, fileB,
												out7, out8,
												renumA, lenA,
												renumB, deltaB, lenB,
												renumSymA, nSymA,
												renumSymB, nSymB) != 0) {
				fprintf(stderr, "[-w] Erreur E8\n");
				goto w_fail;
			}

			printf("[-w] OK: %s\n", out8);

			free(renumA);
			free(renumB);
			free(deltaB);
			free(renumSymA);
			free(renumSymB);

			exit(EXIT_SUCCESS);

		 w_fail:
			free(renumA);
			free(renumB);
			free(deltaB);
			free(renumSymA);
			free(renumSymB);
			exit(EXIT_FAILURE);
		}
		case 'z': {
			const char *out8 = optarg;
			const char *out9 = argv[optind];

			if (!out9) {
				fprintf(stderr, "Erreur: usage -9 out8 out9\n\n");
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}

			if (f) fclose(f);
			f = NULL;

			if (E9_produire_elf_final(out8, out9) != 0) {
				fprintf(stderr, "Erreur: etape 9 a echoue.\n");
				free_Shdr_list(L);
				exit(EXIT_FAILURE);
			}

			printf("E9 OK: %s -> %s\n", out8, out9);
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