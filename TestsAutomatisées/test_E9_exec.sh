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
# test9_exec.sh — Étape 9 : validation EXÉCUTION (+ affichage infos)
#
# Usage:
#   ./test9_exec.sh [out9.o] [ref9.o]
# Defaults:
#   out9.o ref9.o dans le répertoire courant
#
# Tests:
#  (E1) Link final à 0x10000 -> out.elf et ref.elf
#  (E2) Afficher symboles texte: nm | grep " T "
#  (E3) Exécuter 1s sous qemu, capturer 8KB de sortie
#  (E4) out.elf ne crash pas
#  (E5) Afficher un extrait de la sortie (début)
#  (E6) Sortie identique à la référence
# ============================================================

need() {
  command -v "$1" >/dev/null 2>&1 || {
    echo -e "${RED}Outil manquant: $1${NC}" >&2
    exit 2
  }
}

die() {
  echo -e "${RED}ERREUR:$*${NC}" >&2
  exit 1
}

pass() {
  echo -e "${GREEN}[PASS] $*${NC}"
}

info() {
  echo -e "${CYAN}$*${NC}"
}

run() {
  echo -e "${BLUE}→ $*${NC}"
  "$@"
}

# ============================================================
# Vérification outils
# ============================================================
need arm-none-eabi-ld
need arm-none-eabi-nm
need qemu-armeb
need timeout
need head
need diff
need grep
need sed

# ============================================================
# Entrées
# ============================================================
OUT9="${1:-out9.o}"
REF9="${2:-ref9.o}"

[[ -f "$OUT9" ]] || die "Fichier introuvable: $OUT9"
[[ -f "$REF9" ]] || die "Fichier introuvable: $REF9"

OUTELF="out.elf"
REFELF="ref.elf"
OUT_TXT="out_qemu.txt"
REF_TXT="ref_qemu.txt"

rm -f "$OUTELF" "$REFELF" "$OUT_TXT" "$REF_TXT"

# ============================================================
# (E1) Link
# ============================================================
info "=== (E1) Link final (adresse >= 0x10000) ==="
run arm-none-eabi-ld -EB -Ttext 0x10000 -e main -o "$OUTELF" "$OUT9"
run arm-none-eabi-ld -EB -Ttext 0x10000 -e main -o "$REFELF" "$REF9"
pass "ELF générés: $OUTELF et $REFELF"
echo

# ============================================================
# (E2) Symboles
# ============================================================
info "=== (E2) Vérification symboles texte (nm | grep \" T \") ==="
echo "--- out.elf ---"
arm-none-eabi-nm "$OUTELF" | grep " T " || echo "(aucun symbole T trouvé)"
echo "--- ref.elf ---"
arm-none-eabi-nm "$REFELF" | grep " T " || echo "(aucun symbole T trouvé)"
echo

# ============================================================
# Capture QEMU
# ============================================================
capture_run() {
  local elf="$1"
  local outfile="$2"
  set +e
  timeout 1 qemu-armeb "$elf" 2>&1 | head -c 8192 > "$outfile"
  local rc=${PIPESTATUS[0]}
  set -e
  echo "$rc"
}

# ============================================================
# (E3) Exécution
# ============================================================
info "=== (E3) Exécution QEMU (1s) + capture sortie ==="
rc_out="$(capture_run "$OUTELF" "$OUT_TXT")"
rc_ref="$(capture_run "$REFELF" "$REF_TXT")"
pass "Captures effectuées (out rc=$rc_out / ref rc=$rc_ref)"
echo

# ============================================================
# (E4) Crash
# ============================================================
info "=== (E4) Test: pas de crash out.elf ==="
if grep -qiE "illegal instruction|segmentation|abort|core dumped" "$OUT_TXT"; then
  echo -e "${RED}Crash détecté (extrait):${NC}"
  sed -n '1,120p' "$OUT_TXT"
  exit 1
fi
pass "Pas de crash détecté"
echo

# ============================================================
# (E5) Extrait sortie
# ============================================================
info "=== (E5) Extrait de la sortie ==="
sz_ref=$(wc -c < "$REF_TXT" | tr -d ' ')
sz_out=$(wc -c < "$OUT_TXT" | tr -d ' ')

echo "--- ref (${sz_ref} octets) ---"
[[ "$sz_ref" -eq 0 ]] && echo "(aucune sortie)" || sed -n '1,40p' "$REF_TXT"

echo "--- out (${sz_out} octets) ---"
[[ "$sz_out" -eq 0 ]] && echo "(aucune sortie)" || sed -n '1,40p' "$OUT_TXT"
echo

# ============================================================
# (E6) Diff
# ============================================================
info "=== (E6) Test: sortie identique ==="
if ! diff -u "$REF_TXT" "$OUT_TXT" >/dev/null; then
  echo -e "${RED}Sortie différente${NC}"
  diff -u "$REF_TXT" "$OUT_TXT" | sed -n '1,160p'
  exit 1
fi
pass "Sortie identique à la référence"

echo
echo -e "${YELLOW}Codes QEMU: out=$rc_out ref=$rc_ref (124 = timeout)${NC}"
pass "TEST ÉTAPE 9 (EXÉCUTION) : PASS"

rm -f "$OUT_TXT" "$REF_TXT"