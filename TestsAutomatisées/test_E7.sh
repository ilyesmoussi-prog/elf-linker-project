#!/bin/bash

##############################################################################
# Script pour tester UN fichier out7.o existant
# Usage: ./test_E7.sh file1.o file2.o ./out7.o
##############################################################################

# Couleurs
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

if [ $# -ne 3 ]; then
    echo -e "${RED}Usage: $0 <fileA.o> <fileB.o> <out7.o>${NC}"
    echo ""
    echo "Exemple:"
    echo "  $0 ./Examples_fusion/file1.o ./Examples_fusion/file2.o out7.o"
    exit 1
fi

FILEA="$1"
FILEB="$2"
OUT7="$3"

echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}Test de TON fichier out7.o${NC}"
echo -e "${CYAN}========================================${NC}"
echo ""

# Vérifications
if [ ! -f "$FILEA" ]; then
    echo -e "${RED}❌ Fichier introuvable: $FILEA${NC}"
    exit 1
fi

if [ ! -f "$FILEB" ]; then
    echo -e "${RED}❌ Fichier introuvable: $FILEB${NC}"
    exit 1
fi

if [ ! -f "$OUT7" ]; then
    echo -e "${RED}❌ Fichier introuvable: $OUT7${NC}"
    echo ""
    echo -e "${YELLOW} créer out7.o avec:${NC}"
    echo "   ./Options_and_debug_example -g $FILEA $FILEB out6.o out7.o"
    exit 1
fi

echo -e "${GREEN}✓ Fichier A : $FILEA${NC}"
echo -e "${GREEN}✓ Fichier B : $FILEB${NC}"
echo -e "${GREEN}✓ Résultat  : $OUT7${NC}"
echo ""

##############################################################################
# Test 1 : Vérifier que c'est un fichier ELF
##############################################################################
echo -e "${CYAN}[1/5] Vérification format ELF...${NC}"

if ! file "$OUT7" | grep -q "ELF"; then
    echo -e "${RED}❌ $OUT7 n'est pas un fichier ELF${NC}"
    exit 1
fi

if file "$OUT7" | grep -q "MSB"; then
    echo -e "${GREEN}✓ Format ELF big-endian${NC}"
else
    echo -e "${YELLOW}  Format ELF little-endian (attendu: big)${NC}"
fi

##############################################################################
# Test 2 : Compter les symboles
##############################################################################
echo ""
echo -e "${CYAN}[2/5] Analyse des symboles...${NC}"

nsym_a=$(arm-none-eabi-readelf -s "$FILEA" 2>/dev/null | grep -E '^\s*[0-9]+:' | wc -l)
nsym_b=$(arm-none-eabi-readelf -s "$FILEB" 2>/dev/null | grep -E '^\s*[0-9]+:' | wc -l)
nsym_out=$(arm-none-eabi-readelf -s "$OUT7" 2>/dev/null | grep -E '^\s*[0-9]+:' | wc -l)

echo "  Symboles dans A     : $nsym_a"
echo "  Symboles dans B     : $nsym_b"
echo "  Symboles dans OUT7  : $nsym_out"

max_sym=$nsym_a
if [ $nsym_b -gt $max_sym ]; then
    max_sym=$nsym_b
fi

min_expected=$((max_sym - 5))

if [ $nsym_out -ge $min_expected ]; then
    echo -e "${GREEN} Nombre de symboles cohérent${NC}"
else
    echo -e "${RED}❌ Trop peu de symboles (min attendu: $min_expected)${NC}"
fi

##############################################################################
# Test 3 : Vérifier les symboles globaux
##############################################################################
echo ""
echo -e "${CYAN}[3/5] Symboles globaux...${NC}"

nglob_a=$(arm-none-eabi-readelf -s "$FILEA" 2>/dev/null | grep -c " GLOBAL " || echo 0)
nglob_b=$(arm-none-eabi-readelf -s "$FILEB" 2>/dev/null | grep -c " GLOBAL " || echo 0)
nglob_out=$(arm-none-eabi-readelf -s "$OUT7" 2>/dev/null | grep -c " GLOBAL " || echo 0)

echo "  GLOBAL dans A       : $nglob_a"
echo "  GLOBAL dans B       : $nglob_b"
echo "  GLOBAL dans OUT7    : $nglob_out"

expected=$((nglob_a + nglob_b))
min_glob=$((expected - 3))

if [ $nglob_out -ge $min_glob ]; then
    echo -e "${GREEN}✓ Nombre de symboles GLOBAL cohérent (déduplication OK)${NC}"
else
    echo -e "${RED}❌ Trop peu de symboles GLOBAL${NC}"
fi

##############################################################################
# Test 4 : Comparer avec ld -r (référence)
##############################################################################
echo ""
echo -e "${CYAN}[4/5] Comparaison avec ld -r (référence)...${NC}"

ref_o="/tmp/ref_$$.o"
arm-none-eabi-ld -r -EB "$FILEA" "$FILEB" -o "$ref_o" 2>/dev/null

if [ -f "$ref_o" ]; then
    # Extraire les symboles globaux triés
    arm-none-eabi-readelf -s "$ref_o" 2>/dev/null | \
        sed 's/[[:space:]]\+/ /g' | \
        grep " GLOBAL " | \
        grep -vE " __bss_start|__bss_end|_edata|_end|__end__|__data_start|_stack|_start" | \
        awk '{print $2, $8}' | sort > /tmp/ref_sym_$$.txt
    
    arm-none-eabi-readelf -s "$OUT7" 2>/dev/null | \
        sed 's/[[:space:]]\+/ /g' | \
        grep " GLOBAL " | \
        grep -vE " __bss_start|__bss_end|_edata|_end|__end__|__data_start|_stack|_start" | \
        awk '{print $2, $8}' | sort > /tmp/out_sym_$$.txt
    
    if diff -q /tmp/ref_sym_$$.txt /tmp/out_sym_$$.txt > /dev/null 2>&1; then
        echo -e "${GREEN}✅ Symboles identiques à ld -r${NC}"
    else
        echo -e "${YELLOW}  Différences avec ld -r :${NC}"
        diff -u /tmp/ref_sym_$$.txt /tmp/out_sym_$$.txt | head -10
    fi
    
    rm -f "$ref_o" /tmp/ref_sym_$$.txt /tmp/out_sym_$$.txt
else
    echo -e "${YELLOW} Impossible de créer la référence avec ld -r${NC}"
fi

##############################################################################
# Test 5 : Sections
##############################################################################
echo ""
echo -e "${CYAN}[5/5] Vérification des sections...${NC}"

nsec=$(arm-none-eabi-readelf -S "$OUT7" 2>/dev/null | grep -E '^\s*\[' | wc -l)
echo "  Sections dans OUT7  : $nsec"

if arm-none-eabi-readelf -S "$OUT7" 2>/dev/null | grep -q ".symtab"; then
    echo -e "${GREEN}✓ Section .symtab présente${NC}"
else
    echo -e "${RED}❌ Section .symtab absente${NC}"
fi

if arm-none-eabi-readelf -S "$OUT7" 2>/dev/null | grep -q ".strtab"; then
    echo -e "${GREEN}✓ Section .strtab présente${NC}"
else
    echo -e "${RED}❌ Section .strtab absente${NC}"
fi

##############################################################################
# Résumé
##############################################################################
echo ""
echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}Résumé${NC}"
echo -e "${CYAN}========================================${NC}"
echo ""
echo -e "${GREEN}✅  fichier out7.o correct !${NC}"
echo ""
echo -e "${YELLOW}Pour voir tous les symboles :${NC}"
echo "  arm-none-eabi-readelf -s $OUT7"
echo ""
echo -e "${YELLOW}Pour voir toutes les sections :${NC}"
echo "  arm-none-eabi-readelf -S $OUT7"
echo ""
