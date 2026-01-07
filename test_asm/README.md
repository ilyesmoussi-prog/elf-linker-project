#  Fichiers de Test ARM Assembleur

Collection de fichiers assembleur ARM pour tester l'éditeur de liens.

---

##  Liste des Fichiers

###  Tests Simples

| Fichier | Description | Ce qu'il teste |
|---------|-------------|----------------|
| `test_minimal.s` | Fichier ultra-minimal | Pas de relocation, pas de données |
| `test_simple.s` | Fonction simple | Une fonction, symbole local |
| `test_appels.s` | Appels de fonctions | Relocations R_ARM_CALL, symboles externes |
| `test_data.s` | Données et pointeurs | Section .data, relocations R_ARM_ABS32 |
| `test_symboles.s` | Symboles variés | Symboles locaux/globaux, publics/privés |
| `test_rodata.s` | Constantes | Section .rodata (lecture seule) |
| `test_bss.s` | Données non initialisées | Section .bss, grand buffer |
| `test_reloc.s` | Relocations variées | R_ARM_CALL, R_ARM_ABS32, R_ARM_JUMP24 |

###  Tests de Fusion

| Fichier | Description |
|---------|-------------|
| `fusion_a.s` | Fichier A (appelle B) |
| `fusion_b.s` | Fichier B (appelle A) |

Ces deux fichiers s'appellent mutuellement pour tester :
- Résolution des symboles externes
- Fusion des sections .text et .data
- Correction des relocations

---

##  Compilation

### Compiler tous les fichiers
```bash
make
```

### Compiler un fichier spécifique
```bash
make test_simple.o
make test_data.o
```

### Fusionner deux fichiers avec GNU ld
```bash
make fusion
# Crée fusion_ab.o
```

---

##  Examiner les Fichiers

### Voir les informations ELF
```bash
make info-test_simple
make info-test_data
make info-fusion_ab
```

**Affiche :**
- En-tête ELF
- Table des sections
- Table des symboles
- Table des relocations

### Désassembler
```bash
make dis-test_simple
make dis-test_data
```

### Manuellement avec readelf/objdump
```bash
arm-none-eabi-readelf -a test_simple.o
arm-none-eabi-objdump -d test_simple.o
```

---

##  Utiliser avec Votre Éditeur de Liens

### Test Phase 1 (lecture)

```bash
# En-tête
./Options_and_debug_example -t test_simple.o

# Sections
./Options_and_debug_example -S test_data.o

# Symboles
./Options_and_debug_example -s test_symboles.o

# Relocations
./Options_and_debug_example -r test_appels.o

# Dump section
./Options_and_debug_example -x .text test_simple.o
```

### Test Phase 2 (fusion)

```bash
# E6 : Fusion sections
./Options_and_debug_example -f fusion_a.o fusion_b.o out6.o

# E7 : Fusion symboles
./Options_and_debug_example -g fusion_a.o fusion_b.o out6.o out7.o

# E8 : Fusion relocations
./Options_and_debug_example -w fusion_a.o fusion_b.o out8.o

# E9 : Fichier final
./Options_and_debug_example -z out8.o out9.o
```

### Comparer avec GNU ld
```bash
# Votre fusion
./Options_and_debug_example -f fusion_a.o fusion_b.o mon_result.o

# GNU ld
arm-none-eabi-ld -r -mbig-endian -o ld_result.o fusion_a.o fusion_b.o

# Comparer
arm-none-eabi-readelf -a mon_result.o > mon.txt
arm-none-eabi-readelf -a ld_result.o > ld.txt
diff mon.txt ld.txt
```

---

##  Caractéristiques des Fichiers

### test_minimal.s
```
Sections : .text
Symboles : 1 (_start)
Relocations : 0
Taille : ~20 octets
```
**Idéal pour :** Tests de base, vérifier lecture ELF

### test_simple.s
```
Sections : .text
Symboles : 2 (main, .L_local_label)
Relocations : 0
Taille : ~30 octets
```
**Idéal pour :** Symboles locaux/globaux

### test_appels.s
```
Sections : .text
Symboles : 4 (main, fonction_locale, fonction_externe UND)
Relocations : 2 (R_ARM_CALL)
Taille : ~40 octets
```
**Idéal pour :** Relocations CALL, symboles externes

### test_data.s
```
Sections : .text, .data
Symboles : 6 (ma_fonction, ma_variable, tableau, pointeurs)
Relocations : 3 (R_ARM_ABS32)
Taille : ~80 octets
```
**Idéal pour :** Relocations ABS, pointeurs

### test_symboles.s
```
Sections : .text, .data, .bss
Symboles : 7 (4 globaux, 3 locaux)
Relocations : 1
Taille : ~60 octets
```
**Idéal pour :** Fusion symboles, locaux vs globaux

### fusion_a.s + fusion_b.s
```
A : .text + .data, 4 symboles, 2 relocations externes
B : .text + .data, 3 symboles, 2 relocations externes
Fusionnés : .text (A+B), .data (A+B), 6 symboles, 4 relocations
```
**Idéal pour :** Tests complets de fusion

---

##  Cas de Test Spécifiques

### Test 1 : Fichier sans relocation
```bash
./Options_and_debug_example -r test_minimal.o
# Devrait afficher : 0 relocation
```

### Test 2 : Relocations CALL
```bash
./Options_and_debug_example -r test_appels.o
# Devrait afficher : 2 relocations R_ARM_CALL
```

### Test 3 : Relocations ABS32
```bash
./Options_and_debug_example -r test_data.o
# Devrait afficher : 3 relocations R_ARM_ABS32
```

### Test 4 : Section .bss (NOBITS)
```bash
./Options_and_debug_example -S test_bss.o
# Section .bss : type NOBITS, size=1284
./Options_and_debug_example -x .bss test_bss.o
# Devrait gérer section vide (NOBITS)
```

### Test 5 : Fusion complète
```bash
# Compiler
make fusion_a.o fusion_b.o

# Votre fusion
./Options_and_debug_example -f fusion_a.o fusion_b.o my_fusion.o

# Vérifier
./Options_and_debug_example -s my_fusion.o
# Symboles : fonction_a, fonction_b, compteur_a, compteur_b, main

./Options_and_debug_example -r my_fusion.o
# Relocations corrigées avec offsets ajustés
```

---

##  Cas d'Erreur à Tester

### Double définition (doit échouer à E7)
Créer `double_a.s` et `double_b.s` avec même symbole global :
```bash
# double_a.s et double_b.s définissent tous deux "fonction_commune"
./Options_and_debug_example -g double_a.o double_b.o out.o
# Devrait échouer avec erreur : symbole défini plusieurs fois
```

### Symbole non résolu (warning à E8/E9)
```bash
# test_appels.o a des externes non définis
./Options_and_debug_example -f test_appels.o autre.o out.o
# Si "fonction_externe" toujours UND → warning mais pas erreur (fusion partielle OK)
```

---

##  Notes Importantes

### Big Endian
Tous les fichiers sont compilés en **big endian** (`-mbig-endian`), comme requis par le projet.

### Pas de Thumb
Option `-mno-thumb-interwork` pour éviter les complications du mode Thumb.

### Architecture
ARMv7-a (`-march=armv7-a`)

### Format
ELF 32-bit MSB (Most Significant Byte first)

---

##  Dépannage

### Erreur : arm-none-eabi-as: command not found
```bash
# Installer la toolchain ARM
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi
```

### Vérifier qu'un .o est bien big endian
```bash
file test_simple.o
# Devrait afficher : ELF 32-bit MSB executable, ARM
```

### Voir les relocations détaillées
```bash
arm-none-eabi-readelf -r test_appels.o
```

---

##  Utilisation Pédagogique

Ces fichiers couvrent progressivement :
1.  Fichiers ELF basiques (test_minimal)
2.  Symboles et sections (test_simple, test_symboles)
3.  Relocations simples (test_appels)
4.  Données et pointeurs (test_data)
5.  Fusion complète (fusion_a + fusion_b)

**Suivre cet ordre pour tester progressivement votre éditeur !**

---

##  Fichiers Fournis

```
test_asm/
├── Makefile              # Compilation automatique
├── README.md            # Ce fichier
├── test_minimal.s       # Fichier ultra-simple
├── test_simple.s        # Fonction basique
├── test_appels.s        # Appels externes
├── test_data.s          # Données et relocations ABS
├── test_symboles.s      # Symboles variés
├── test_rodata.s        # Constantes (read-only)
├── test_bss.s           # Section BSS
├── test_reloc.s         # Relocations variées
├── fusion_a.s           # Fichier A pour fusion
└── fusion_b.s           # Fichier B pour fusion
```

---

##  Résumé

**10 fichiers de test** couvrant tous les aspects :
-  Sections : .text, .data, .bss, .rodata
-  Symboles : locaux, globaux, externes (UND)
-  Relocations : R_ARM_CALL, R_ARM_ABS32, R_ARM_JUMP24
-  Fusion : tests complets de fusion de 2 fichiers

**Utilisez le Makefile pour tout automatiser !**

```bash
make              # Compile tout
make help         # Aide
make info-XXX     # Infos sur un fichier
```

**Prêt à tester votre éditeur de liens !** 
