#!/bin/bash

##############################################################################
# Autotest Phase 1 - Version Complète
# Teste une option sur tous les fichiers d'un dossier
# Compare avec readelf en utilisant diff
##############################################################################

# Couleurs
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Valeurs par défaut
DOSSIER="./Examples_fusion"
PROGRAMME="./Options_and_debug_example"
OPTION="-t"
ARG_SUPP=""  # Pour -x

##############################################################################
# Usage
##############################################################################
usage() {
    echo -e "${CYAN}Usage: $0 [-d dossier] [-o option] [-a arg]${NC}"
    echo ""
    echo -e "${YELLOW}Options:${NC}"
    echo "  -d dossier    Dossier contenant les fichiers .o (défaut: ./Examples_fusion)"
    echo "  -o option     Option à tester: -t, -S, -s, -r, -x (défaut: -t)"
    echo "  -a arg        Argument pour -x (nom de section)"
    echo "  -h            Afficher cette aide"
    echo ""
    echo -e "${YELLOW}Exemples:${NC}"
    echo "  $0                              # Test -t sur *.o"
    echo "  $0 -o -S                        # Test -S (sections)"
    echo "  $0 -o -s -d ./test_asm          # Test -s sur ./test_asm"
    echo "  $0 -o -x -a .text               # Test -x .text"
    echo "  $0 -o -r                        # Test -r (relocations)"
    exit 0
}

##############################################################################
# Parsing
##############################################################################
while getopts "d:o:a:h" opt; do
    case $opt in
        d) DOSSIER="$OPTARG" ;;
        o) OPTION="$OPTARG" ;;
        a) ARG_SUPP="$OPTARG" ;;
        h) usage ;;
        *) usage ;;
    esac
done

normalize_x() {
  awk '
    /^\s*0x[0-9a-f]+/ {
      addr=$1
      out=addr
      # garde uniquement les champs hex après l’adresse
      for (i=2; i<=NF; i++) {
        if ($i ~ /^[0-9a-f]+$/) out = out " " $i
        else break
      }
      print out
    }'
}


##############################################################################
# Nom de l'option
##############################################################################
case "$OPTION" in
    -t) NOM_OPTION="En-tête ELF" ;;
    -S) NOM_OPTION="Table des Sections" ;;
    -x) NOM_OPTION="Contenu de Section" ;;
    -s) NOM_OPTION="Table des Symboles" ;;
    -r) NOM_OPTION="Table de Relocation" ;;
    *) echo "Option invalide: $OPTION"; exit 1 ;;
esac

##############################################################################
# Vérifications
##############################################################################
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Autotest Phase 1 - $NOM_OPTION${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

if [ ! -x "$PROGRAMME" ]; then
    echo -e "${RED}❌ Programme introuvable: $PROGRAMME${NC}"
    exit 1
fi

if [ ! -d "$DOSSIER" ]; then
    echo -e "${RED}❌ Dossier introuvable: $DOSSIER${NC}"
    exit 1
fi

if [ "$OPTION" = "-x" ] && [ -z "$ARG_SUPP" ]; then
    echo -e "${RED}❌ L'option -x nécessite -a (nom de section)${NC}"
    echo "Exemple: $0 -o -x -a .text"
    exit 1
fi

echo -e "${CYAN}Programme :${NC} $PROGRAMME"
echo -e "${CYAN}Dossier  :${NC} $DOSSIER"
echo -e "${CYAN}Option   :${NC} $OPTION ($NOM_OPTION)"
[ -n "$ARG_SUPP" ] && echo -e "${CYAN}Section  :${NC} $ARG_SUPP"
echo ""

##############################################################################
# Collecter les fichiers .o
##############################################################################
FICHIERS=()
for f in "$DOSSIER"/*.o; do
    [ -e "$f" ] && FICHIERS+=("$f")
done

if [ ${#FICHIERS[@]} -eq 0 ]; then
    echo -e "${RED}❌ Aucun fichier .o trouvé dans $DOSSIER${NC}"
    exit 1
fi

echo -e "${GREEN}✓ ${#FICHIERS[@]} fichier(s) .o trouvé(s)${NC}"
echo ""

##############################################################################
# Tests
##############################################################################
TOTAL=0
SUCCES=0
ECHECS=0

for FILE in "${FICHIERS[@]}"; do
    TOTAL=$((TOTAL + 1))
    nom=$(basename "$FILE")
    
    # Fichiers temporaires
    MINE="/tmp/mine_$$.txt"
    READELF_OUT="/tmp/readelf_$$.txt"
    
    printf "%-40s : " "$nom"
    
    ##########################################################################
    # Comparaison selon l'option
    ##########################################################################
    case "$OPTION" in
        -t)
            $PROGRAMME -t "$FILE" | tail -n 19 > "$MINE" 2>/dev/null
            arm-none-eabi-readelf -h "$FILE" | tail -n 19 > "$READELF_OUT" 2>/dev/null
            ;;
            
        -S)
            $PROGRAMME -S "$FILE" \
              | grep -E '^[[:space:]]*\[' \
              | tr -s ' ' \
              > "$MINE" 2>/dev/null
            arm-none-eabi-readelf --wide -S "$FILE" \
              | grep -E '^[[:space:]]*\[' \
              | tr -s ' ' \
              > "$READELF_OUT" 2>/dev/null
            ;;
            
        -s)
            $PROGRAMME -s "$FILE" \
              | grep -E '^[[:space:]]*[0-9]+' \
              | tr -s ' ' \
              > "$MINE" 2>/dev/null
            arm-none-eabi-readelf --wide -s "$FILE" \
              | grep -E '^[[:space:]]*[0-9]+' \
              | tr -s ' ' \
              > "$READELF_OUT" 2>/dev/null
            ;;
            
        -x)
            $PROGRAMME -x "$ARG_SUPP" "$FILE" 2>/dev/null | normalize_x > "$MINE"

            arm-none-eabi-readelf -x "$ARG_SUPP" "$FILE" 2>/dev/null | normalize_x > "$READELF_OUT"
            ;;

        
        -r)
            $PROGRAMME -r "$FILE" \
              | grep -E '^\s+[0-9a-f]{8}' \
              | tr -s ' ' \
              | awk '{print $1, $2, $3}' \
              | sed 's/\s\+$//' \
              > "$MINE" 2>/dev/null
            arm-none-eabi-readelf -r "$FILE" \
              | grep -E '^\s*[0-9a-f]{8}' \
              | tr -s ' ' \
              | awk '{
                  offset = $1
                  type = $3
                  info = $2
                  sym_hex = substr(info, 1, length(info)-2)
                  sym_num = strtonum("0x" sym_hex)
                  print offset, type, sym_num
              }' \
              | sed 's/\s\+$//' \
              > "$READELF_OUT" 2>/dev/null
            ;;
    esac
    
    ##########################################################################
    # Comparaison avec diff
    ##########################################################################
    if diff -u -w "$READELF_OUT" "$MINE" > /dev/null 2>&1; then
        echo -e "${GREEN}✅ OK${NC}"
        SUCCES=$((SUCCES + 1))
    else
        echo -e "${RED}❌ FAIL${NC}"
        ECHECS=$((ECHECS + 1))
        
        # Afficher le diff pour ce fichier (optionnel)
        # diff -u "$READELF_OUT" "$MINE" | head -20
    fi
    
    rm -f "$MINE" "$READELF_OUT"
done

##############################################################################
# Résumé
##############################################################################
echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "Total  : $TOTAL"
echo -e "${GREEN}Succès : $SUCCES${NC}"

if [ $ECHECS -gt 0 ]; then
    echo -e "${RED}Échecs : $ECHECS${NC}"
fi

if [ $TOTAL -gt 0 ]; then
    TAUX=$((SUCCES * 100 / TOTAL))
    echo -e "Taux   : ${CYAN}$TAUX%${NC}"
fi

echo ""

if [ $ECHECS -eq 0 ]; then
    echo -e "${GREEN}🎉 Tous les tests ont réussi !${NC}"
    exit 0
else
    echo -e "${RED}⚠️  $ECHECS test(s) ont échoué${NC}"
    echo ""
    echo -e "${YELLOW}💡 Pour voir les détails d'un fichier :${NC}"
    echo "   ./phase1_autotest.sh -o $OPTION <fichier.o>"
    exit 1
fi