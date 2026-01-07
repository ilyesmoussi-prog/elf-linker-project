#!/usr/bin/env bash
set -euo pipefail

# ============================================================
# Couleurs ANSI
# ============================================================
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# ============================================================
# test9_o.sh — Étape 9 : génération + tests STRUCTURELS sur out9.o
#
# Usage:
#   ./test9_o.sh <A.o> <B.o>
#
# Génère: out6.o out7.o out8.o out9.o + ref9.o
# Puis teste:
#  (T1) out9.o est ELF32 / big-endian / REL / ARM
#  (T2) readelf/objdump lisent sections/symboles/relocs sans erreur
#  (T3) .shstrtab existe (reconstruite en E9)
#  (T4) .symtab et .strtab existent
#  (T5) .symtab sh_link -> .strtab
#  (T6) .symtab sh_info = index du 1er symbole non-local
#  (T7) Chaque section REL a sh_link -> .symtab
#  (T8) Comparaison NORMALISÉE vs ref9.o (ld -r), en ignorant debug/comment/note
# ============================================================
#pour un bon affichage visuel
need() {
  command -v "$1" >/dev/null 2>&1 || {
    echo -e "${RED}Outil manquant: $1${NC}" >&2
    exit 2
  }
}
die()  { echo -e "${RED}$*${NC}" >&2; exit 1; }
pass() { echo -e "${GREEN}$*${NC}"; }

[[ $# -eq 2 ]] || die "Usage: ./test9_o.sh <A.o> <B.o>"
A="$1"
B="$2"

[[ -f "$A" ]] || die "Fichier introuvable: $A"
[[ -f "$B" ]] || die "Fichier introuvable: $B"
[[ -x ./Options_and_debug_example ]] || die "./Options_and_debug_example introuvable ou non exécutable"

need arm-none-eabi-readelf
need arm-none-eabi-ld
need arm-none-eabi-objdump
need awk
need grep
need sed
need sort
need diff

OUT6="out6.o"
OUT7="out7.o"
OUT8="out8.o"
OUT9="out9.o"
REF9="ref9.o"

run() { echo -e "${BLUE}→ $*${NC}"; "$@"; }

# Nettoyage simple (évite confusion)
rm -f "$OUT6" "$OUT7" "$OUT8" "$OUT9" "$REF9" \
      S.mine S.ref s.mine s.ref r.mine r.ref

echo -e "${CYAN}=== (1) Génération out9.o (E6/E7/E8/E9) ===${NC}"
run ./Options_and_debug_example -f "$A" "$B" "$OUT6"
run ./Options_and_debug_example -g "$A" "$B" "$OUT6" "$OUT7"
run ./Options_and_debug_example -w "$A" "$B" "$OUT8"
run ./Options_and_debug_example -z "$OUT8" "$OUT9"
pass "[PASS] out9.o généré: $OUT9"
echo

echo -e "${CYAN}=== (2) Référence GNU (ld -r) => ref9.o ===${NC}"
run arm-none-eabi-ld -r -EB -o "$REF9" "$A" "$B"
pass "[PASS] ref9.o généré: $REF9"
echo

# Fonctions d'extraction infos section
sec_line() {
  # Renvoie: idx name type link info align
  arm-none-eabi-readelf -SW "$1" | awk -v NAME="$2" '
    /^[[:space:]]*\[/{
      if ($1=="[") { idx=$2; gsub(/\]/,"",idx); name=$3; type=$4; }
      else { idx=$1; gsub(/^\[/,"",idx); gsub(/\]$/,"",idx); name=$2; type=$3; }
      link=$(NF-2); info=$(NF-1); align=$NF;
      if (name==NAME) { print idx, name, type, link, info, align; exit 0; }
    }'
}
sec_idx()  { sec_line "$1" "$2" | awk '{print $1}'; }
sec_link() { sec_line "$1" "$2" | awk '{print $4}'; }
sec_info() { sec_line "$1" "$2" | awk '{print $5}'; }

echo -e "${CYAN}=== (3) Tests STRUCTURELS sur out9.o ===${NC}"

# (T1) Header attendu
HDR="$(arm-none-eabi-readelf -h "$OUT9")"
echo "$HDR" | grep -q "Class: *ELF32" || die "(T1) Pas ELF32"
echo "$HDR" | grep -q "Data: *2's complement, big endian" || die "(T1) Pas big-endian"
echo "$HDR" | grep -q "Type: *REL (Relocatable file)" || die "(T1) Pas Type REL"
echo "$HDR" | grep -q "Machine: *ARM" || die "(T1) Pas Machine ARM"
pass "[PASS] (T1) Header OK: ELF32 / big-endian / REL / ARM"

# (T2) Lisibilité tables (pas corrompues)
arm-none-eabi-readelf -S "$OUT9" >/dev/null || die "(T2) readelf -S échoue"
arm-none-eabi-readelf -s "$OUT9" >/dev/null || die "(T2) readelf -s échoue"
arm-none-eabi-readelf -r "$OUT9" >/dev/null || die "(T2) readelf -r échoue"
arm-none-eabi-objdump -dr "$OUT9" >/dev/null || die "(T2) objdump -dr échoue"
pass "[PASS] (T2) Tables lisibles (sections/symboles/relocations)"

# (T3) .shstrtab doit exister
arm-none-eabi-readelf -SW "$OUT9" | grep -q "\.shstrtab" || die "(T3) .shstrtab absente"
pass "[PASS] (T3) .shstrtab présente"

# (T4) .symtab et .strtab existent
SYMIDX="$(sec_idx "$OUT9" ".symtab")"
STRIDX="$(sec_idx "$OUT9" ".strtab")"
[[ -n "$SYMIDX" ]] || die "(T4) .symtab absente"
[[ -n "$STRIDX" ]] || die "(T4) .strtab absente"
pass "[PASS] (T4) .symtab/.strtab présentes (symtab=$SYMIDX, strtab=$STRIDX)"

# (T5) .symtab sh_link -> .strtab
SYMLINK="$(sec_link "$OUT9" ".symtab")"
[[ "$SYMLINK" == "$STRIDX" ]] || die "(T5) .symtab sh_link=$SYMLINK (attendu $STRIDX)"
pass "[PASS] (T5) .symtab sh_link -> .strtab OK"

# (T6) .symtab sh_info = index du 1er symbole non-local
FIRST_NONLOCAL="$(
  arm-none-eabi-readelf -sW "$OUT9" | awk '
    BEGIN{found=0}
    /^ *[0-9]+:/{
      num=$1; sub(/:/,"",num);
      bind=$5;
      if (bind!="LOCAL") { print num; found=1; exit }
    }
    END{ if(!found) print 0 }
  '
)"

SYMINFO="$(sec_info "$OUT9" ".symtab")"
[[ "$SYMINFO" == "$FIRST_NONLOCAL" ]] || die "(T6) .symtab sh_info=$SYMINFO (attendu $FIRST_NONLOCAL)"
pass "[PASS] (T6) .symtab sh_info OK (= 1er non-local)"

# (T7) Toutes les sections REL doivent avoir sh_link -> .symtab
BAD_REL="$(
  arm-none-eabi-readelf -SW "$OUT9" | awk -v SYMIDX="$SYMIDX" '
    /^[[:space:]]*\[/{
      if ($1=="[") { name=$3; type=$4; }
      else { name=$2; type=$3; }
      link=$(NF-2);
      if (type=="REL" && link!=SYMIDX) { print name " link=" link " attendu=" SYMIDX; bad=1; }
    }
    END{ if(bad) exit 0; else exit 1; }' || true
)"
[[ -z "$BAD_REL" ]] || { echo -e "${RED}(T7) Mauvais sh_link pour REL:${NC}"; echo "$BAD_REL"; exit 1; }
pass "[PASS] (T7) Toutes les sections REL pointent vers .symtab"