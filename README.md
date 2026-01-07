README - ELF Linker pédagogique

Prérequis
- OS : Linux
- Outils : gcc, make

Compilation
1. Depuis le répertoire racine du projet :
```sh
 autoreconf -vif
./configure && make
```

Le binaire produit : `Options_and_debug_example`.

Usage général
```sh
Options_and_debug_example [options]
Options_and_debug_example --help
```

Options utiles
- `-t <file>` : afficher l'en-tête ELF
- `-S <file>` : afficher la table des sections
- `-x <section> <file>` : afficher le contenu d'une section (nom ou index)
- `-s <file>` : afficher la table des symboles
- `-r <file>` : afficher la table des relocations

Flux de fusion (multi-étapes)
- `-f fileA fileB out` : étape 6 — fusionner les sections (écrit `out`)
- `-g fileA fileB out6 out7` : étape 7 — fusion/correction des symboles après étape6
- `-w fileA fileB out8` : étapes 6+7+8 (écrit `out8`)
- `-z out8 out9` : étape 9 — produire ELF final normalisé (`out9`)

Comportement important
- Le code cible ELF32 (pas ELF64).
- Les relocations de type RELA et les tables dynamiques (`.dynsym`/`.dynstr`) ne sont pas entièrement prises en charge.

Tests (fournis)
Dans `TestsAutomatisées` il existe des scripts/fichiers de test :
- `test_phase1.sh` : tests pour les fonctions d'affichage et lecture (phase 1)
- `test_E6` : ressources/tests pour l'étape 6
- `test_E7.sh` : script/tests pour l'étape 7
- `test_E8` : ressources/tests pour l'étape 8
- `test_E9.sh`, `test_E9_exec.sh` : tests pour l'étape 9

Exemples de commandes de test
```sh
# Exécuter les tests d'affichage/lecture (phase 1)
chmod +x TestsAutomatisées/*
bash TestsAutomatisées/test_phase1.sh

# Lancer le script de l'étape 7
bash TestsAutomatisées/test_E7.sh

# Exemples manuels pour vérifier E6..E9 :
./Options_and_debug_example -f a.o b.o out6.o
./Options_and_debug_example -g a.o b.o out6.o out7.o
./Options_and_debug_example -w a.o b.o out8.o
./Options_and_debug_example -z out8.o out9.o
```

Dépannage rapide
- Vérifier que les fichiers d'entrée existent et sont des objets ELF32.
- Lire stderr pour les messages d'erreur.

Souhaitez-vous que je commite ce `README.md` dans le dépôt ?