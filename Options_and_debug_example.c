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
		" [ -S file ] : Affiche la table des sections ELF\n",
		name);
}


int main(int argc, char *argv[]) {
	int opt;

	struct option longopts[] = {
		{ "debug", required_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};

	while ((opt = getopt_long(argc, argv, "d:h:t:S", longopts, NULL)) != -1) {
		switch(opt) {
		case 't':
			if (optind >= argc) {
				fprintf(stderr, "Erreur: fichier manquant\n\n");
				usage(argv[0]);
				exit(1);
			}

			FILE *f = fopen(argv[optind], "rb");
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
			if (optind >= argc) {
				fprintf(stderr, "Erreur: fichier manquant\n\n");
				usage(argv[0]);
				exit(1);
			}	
			FILE *fs = fopen(argv[optind], "rb");
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