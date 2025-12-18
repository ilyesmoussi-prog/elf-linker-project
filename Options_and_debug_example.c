/*
ELF Loader - chargeur/implanteur d'exécutables au format ELF à but pédagogique
Copyright (C) 2012 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique Générale GNU publiée par la Free Software
Foundation (version 2 ou bien toute autre version ultérieure choisie par vous).

Ce programme est distribué car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but spécifique. Reportez-vous à la
Licence Publique Générale GNU pour plus de détails.

Vous devez avoir reçu une copie de la Licence Publique Générale GNU en même
temps que ce programme ; si ce n'est pas le cas, écrivez à la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
États-Unis.

Contact: Guillaume.Huard@imag.fr
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
		" [ -b file ] : Affiche la table des symboles ELF\n"
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

	while ((opt = getopt_long(argc, argv, "d:h:t:S:x:b:r:", longopts, NULL)) != -1) {
		switch(opt) {
		case 't':
			FILE *f = fopen(optarg, "rb");
			if (f == NULL) {
				perror("fopen");
				exit(EXIT_FAILURE);
			}

			Elf32_Ehdr hdr;
			if (read_Elf32_Ehdr(f, &hdr) != 0) {
				fprintf(stderr, "Erreur: impossible de lire l'en-tete ELF.\n");
				fclose(f);
				return EXIT_FAILURE;
			}
			afficher_read_Elf32_Ehdr(hdr);
			fclose(f);
			break;
		case 'S':
			FILE *fs = fopen(optarg, "rb");
			if (fs == NULL) {
				perror("fopen");
				exit(EXIT_FAILURE);
			}	
			Elf32_Ehdr hdrs;
			if (read_Elf32_Ehdr(fs, &hdrs) != 0) {
				fprintf(stderr, "Erreur: impossible de lire l'en-tete ELF.\n");
				fclose(fs);
				return EXIT_FAILURE;
			}
			Shdr_liste *L = malloc(sizeof(Shdr_liste));
			if (L == NULL) {
				perror("malloc");
				fclose(fs);
				return EXIT_FAILURE;
			}
			read_Shdr_list(fs, hdrs, L);
			afficher_Shdr_list(fs, hdrs, L);
			fclose(fs);
			break;
		case 'b': {
			FILE *f1 = fopen(optarg, "rb");
			if (f1 == NULL) {
				perror("fopen");
				exit(EXIT_FAILURE);
			}

			Elf32_Ehdr h;
			if (read_Elf32_Ehdr(f1, &h) != 0) {
				fprintf(stderr, "Erreur: impossible de lire l'en-tete ELF.\n");
				fclose(f1);
				exit(EXIT_FAILURE);
			}

			Shdr_liste *L1 = malloc(sizeof(Shdr_liste));
			if (L1 == NULL) {
				perror("malloc");
				fclose(f1);
				exit(EXIT_FAILURE);
			}

			read_Shdr_list(f1, h, L1);
			afficher_symtab(f1, h, L1);

			fclose(f1);
			break;
		}	
		case 'x': {
			/*if (optind >= argc) {
				fprintf(stderr, "Erreur: fichier manquant\n\n");
				usage(argv[0]);
				exit(1);
			}*/
			FILE *fx = fopen(optarg, "rb");
			if (fx == NULL) {
				perror("fopen");
				exit(EXIT_FAILURE);
			}

			Elf32_Ehdr hdrx;
			if (read_Elf32_Ehdr(fx, &hdrx) != 0) {
				fprintf(stderr, "Erreur: impossible de lire l'en-tete ELF.\n");
				fclose(fx);
				exit(EXIT_FAILURE);
			}

			Shdr_liste *Lx = malloc(sizeof(Shdr_liste));
			if (Lx == NULL) {
				perror("malloc");
				fclose(fx);
				exit(EXIT_FAILURE);
			}

			read_Shdr_list(fx, hdrx, Lx);
			const char *secname=optarg;
			/* chercher la section .text */
			Shdr_liste *section = section_name(fx, hdrx, Lx, secname);
			//Shdr_liste *section = section_index(Lx, secname); // suppose que .text est la section 1
			if (section == NULL) {
				fprintf(stderr, "Section .text introuvable\n");
				fclose(fx);
				exit(EXIT_FAILURE);
			}

			afficher_content_section(section);

			fclose(fx);
			break;
		}
		case 'r': {
			FILE *fr = fopen(optarg, "rb");
			if (fr == NULL) {
				perror("fopen");
				exit(EXIT_FAILURE);
			}

			Elf32_Ehdr hr;
			if (read_Elf32_Ehdr(fr, &hr) != 0) {
				fprintf(stderr, "Erreur: impossible de lire l'en-tete ELF.\n");
				fclose(fr);
				exit(EXIT_FAILURE);
			}

			Shdr_liste *Lr = malloc(sizeof(Shdr_liste));
			if (Lr == NULL) {
				perror("malloc");
				fclose(fr);
				exit(EXIT_FAILURE);
			}

			read_Shdr_list(fr, hr, Lr);
			//afficher la table de relocation
			afficher_relocation(hr, Lr);

			fclose(fr);
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
	}

	return 0;
}