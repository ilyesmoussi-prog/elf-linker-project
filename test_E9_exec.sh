#!/usr/bin/env bash
set -euo pipefail

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

need() { command -v "$1" >/dev/null 2>&1 || { echo " Outil manquant: $1" >&2; exit 2; }; }
die()  { echo " ❌$*" >&2; exit 1; }
pass()   { echo "✅ $*"; }

need arm-none-eabi-ld
need arm-none-eabi-nm
need qemu-armeb
need timeout
need head
need diff
need grep
need sed

OUT9="${1:-out9.o}"
REF9="${2:-ref9.o}"
[[ -f "$OUT9" ]] || die "Fichier introuvable: $OUT9"
[[ -f "$REF9" ]] || die "Fichier introuvable: $REF9"

OUTELF="out.elf"
REFELF="ref.elf"
OUT_TXT="out_qemu.txt"
REF_TXT="ref_qemu.txt"

run() { echo "→ $*"; "$@"; }

rm -f "$OUTELF" "$REFELF" "$OUT_TXT" "$REF_TXT"

echo "=== (E1) Link final (adresse >= 0x10000) ==="
run arm-none-eabi-ld -EB -Ttext 0x10000 -e main -o "$OUTELF" "$OUT9"
run arm-none-eabi-ld -EB -Ttext 0x10000 -e main -o "$REFELF" "$REF9"
pass "ELF générés: $OUTELF et $REFELF"
echo

echo "=== (E2) Vérification symboles texte (nm | grep \" T \") ==="
echo "--- out.elf ---"
arm-none-eabi-nm "$OUTELF" | grep " T " || echo "(aucun symbole T trouvé)"
echo "--- ref.elf ---"
arm-none-eabi-nm "$REFELF" | grep " T " || echo "(aucun symbole T trouvé)"
echo

capture_run() {
  local elf="$1"
  local outfile="$2"
  set +e
  timeout 1 qemu-armeb "$elf" 2>&1 | head -c 8192 > "$outfile"
  local rc=${PIPESTATUS[0]}
  set -e
  echo "$rc"
}

echo "=== (E3) Exécution QEMU (1s) + capture sortie ==="
rc_out="$(capture_run "$OUTELF" "$OUT_TXT")"
rc_ref="$(capture_run "$REFELF" "$REF_TXT")"
pass "Captures: $OUT_TXT / $REF_TXT (rc_out=$rc_out, rc_ref=$rc_ref)"
echo

echo "=== (E4) Test: pas de crash out.elf ==="
if grep -qiE "illegal instruction|segmentation|abort|core dumped" "$OUT_TXT"; then
  echo " Crash détecté (extrait):"
  sed -n '1,120p' "$OUT_TXT"
  exit 1
fi
pass "Pas de crash détecté"
echo

echo "=== (E5) Affichage d'un extrait de la sortie capturée ==="
sz_ref=$(wc -c < "$REF_TXT" | tr -d ' ')
sz_out=$(wc -c < "$OUT_TXT" | tr -d ' ')

echo "--- ref (taille ${sz_ref} octets) ---"
if [[ "$sz_ref" -eq 0 ]]; then
  echo "(aucune sortie)"
else
  sed -n '1,40p' "$REF_TXT"
fi

echo "--- out (taille ${sz_out} octets) ---"
if [[ "$sz_out" -eq 0 ]]; then
  echo "(aucune sortie)"
else
  sed -n '1,40p' "$OUT_TXT"
fi
echo
echo "=== (E6) Test: sortie identique à la référence ==="
if ! diff -u "$REF_TXT" "$OUT_TXT" >/dev/null; then
  echo " Sortie différente"
  echo "Diff (premières lignes):"
  diff -u "$REF_TXT" "$OUT_TXT" | sed -n '1,160p'
  exit 1
fi
pass "Sortie identique à la référence"

echo
echo "  Codes QEMU: out=$rc_out ref=$rc_ref (124 = timeout si le programme boucle)"
pass "TEST ÉTAPE 9 (EXÉCUTION) : PASS ✅"
rm -f "$OUT_TXT" "$REF_TXT"