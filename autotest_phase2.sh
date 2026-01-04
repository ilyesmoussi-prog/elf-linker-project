#!/bin/bash

OPTION="$1"
FILEA="$2"
FILEB="$3"
OUTFILE="$4"

if [ -z "$OPTION" ] || [ -z "$FILEA" ]; then
    echo "Usage:"
    echo "  $0 -f <fileA.o> <fileB.o> <out.o>      # Test étape 6 (fusion sections)"
    exit 1
fi

SECA="/tmp/sections_A.txt"
SECB="/tmp/sections_B.txt"
SECU="/tmp/sections_union.txt"

fail() { echo " FAIL: $*"; exit 1; }
pass() { echo " PASS: $*"; }

# ========== FONCTIONS UTILITAIRES ==========

liste_sections_utiles() {
  F="$1"
  arm-none-eabi-readelf -S --wide "$F" \
    | awk '
      /^\s*\[\s*[0-9]+\]/ {
        name=$2; type=$3; size=$6;
        if (name=="") next;
        if (name ~ /^\.debug/) next;
        if (name ~ /^\.rel/ || name ~ /^\.rela/) next;
        if (type=="REL" || type=="RELA") next;
        print name, type, size;
      }'
}

get_type() {
  LIST="$1"; NAME="$2"
  awk -v n="$NAME" '$1==n {print $2; exit}' "$LIST"
}

get_size_hex() {
  LIST="$1"; NAME="$2"
  awk -v n="$NAME" '$1==n {print $3; exit}' "$LIST"
}

hex_to_dec() {
  H="$1"
  # Enlever le préfixe 0x si présent
  H=$(echo "$H" | sed 's/^0x//')
  # Convertir hex -> dec (compatible bash)
  printf "%d" "0x$H"
}

section_existe() {
  F="$1"; NAME="$2"
  arm-none-eabi-readelf -S --wide "$F" | grep -qE "[[:space:]]$NAME([[:space:]]|$)"
}

dump_hex_stream() {
  F="$1"; NAME="$2"
  arm-none-eabi-readelf -x "$NAME" "$F" 2>/dev/null \
    | awk '
      /^\s*0x[0-9a-fA-F]+/ {
        for (i=2; i<=NF; i++) {
          if ($i ~ /^[0-9a-fA-F]{8}$/) printf "%s", $i;
        }
      }
      END { printf "\n"; }'
}

# ========== TESTS ==========

case "$OPTION" in
  -f)
    [ -n "$FILEB" ] || fail "Usage: $0 -f <fileA.o> <fileB.o> <out.o>"
    [ -n "$OUTFILE" ] || fail "Usage: $0 -f <fileA.o> <fileB.o> <out.o>"

    echo "== Autotest ETAPE 6 (Fusion sections) =="

    ./Options_and_debug_example -f "$FILEA" "$FILEB" "$OUTFILE" || fail "Fusion -f a échoué"
    [ -f "$OUTFILE" ] || fail "Fichier sortie introuvable: $OUTFILE"
    pass "Fichier sortie créé: $OUTFILE"

    arm-none-eabi-readelf -S --wide "$OUTFILE" 2>&1 | grep -q "Warning:" && fail "readelf -S affiche des warnings"
    pass "readelf -S sans warnings"

    liste_sections_utiles "$FILEA" > "$SECA"
    liste_sections_utiles "$FILEB" > "$SECB"
    awk '{print $1}' "$SECA" "$SECB" | sort -u > "$SECU"

    echo
    echo "== Tests par section (règles étape 6) =="

    while IFS= read -r secname; do
      [ -n "$secname" ] || continue

      if [ "$secname" = ".shstrtab" ]; then
        section_existe "$OUTFILE" ".shstrtab" || fail ".shstrtab absente"
        pass ".shstrtab présente (reconstruite)"
        continue
      fi

      typeA=$(get_type "$SECA" "$secname")
      typeB=$(get_type "$SECB" "$secname")

      inA=0; inB=0
      [ -n "$typeA" ] && inA=1
      [ -n "$typeB" ] && inB=1

      section_existe "$OUTFILE" "$secname" || fail "Section manquante: $secname"

      out_size_hex=$(arm-none-eabi-readelf -S --wide "$OUTFILE" | awk -v s="$secname" '$0 ~ ("[[:space:]]" s "([[:space:]]|$)") {print $6; exit}')
      [ -n "$out_size_hex" ] || fail "Impossible de lire taille: $secname"

      # CAS 1: Section dans A et B
      if [ "$inA" -eq 1 ] && [ "$inB" -eq 1 ]; then
        sizeA_hex=$(get_size_hex "$SECA" "$secname")
        sizeB_hex=$(get_size_hex "$SECB" "$secname")
        sizeA_dec=$(hex_to_dec "$sizeA_hex")
        sizeB_dec=$(hex_to_dec "$sizeB_hex")
        out_dec=$(hex_to_dec "$out_size_hex")

        # PROGBITS: concat A||B
        if [ "$typeA" = "PROGBITS" ] && [ "$typeB" = "PROGBITS" ]; then
          expect=$((sizeA_dec + sizeB_dec))
          [ "$out_dec" -eq "$expect" ] || fail "$secname: taille != A+B"

          ha=$(dump_hex_stream "$FILEA" "$secname")
          hb=$(dump_hex_stream "$FILEB" "$secname")
          ho=$(dump_hex_stream "$OUTFILE" "$secname")

          [ "$ho" = "${ha}${hb}" ] || fail "$secname: contenu != concat(A||B)"
          pass "$secname (PROGBITS): concat OK"
          continue
        fi

        # NOBITS: somme tailles
        if [ "$typeA" = "NOBITS" ] && [ "$typeB" = "NOBITS" ]; then
          expect=$((sizeA_dec + sizeB_dec))
          [ "$out_dec" -eq "$expect" ] || fail "$secname (NOBITS): taille != A+B"
          pass "$secname (NOBITS): taille OK"
          continue
        fi

        # Autres: garde A
        [ "$out_dec" -eq "$sizeA_dec" ] || fail "$secname: taille != A"
        pass "$secname: garde A"
        continue
      fi

      # CAS 2: Uniquement A
      if [ "$inA" -eq 1 ] && [ "$inB" -eq 0 ]; then
        sizeA_hex=$(get_size_hex "$SECA" "$secname")
        sizeA_dec=$(hex_to_dec "$sizeA_hex")
        out_dec=$(hex_to_dec "$out_size_hex")
        [ "$out_dec" -eq "$sizeA_dec" ] || fail "$secname: taille != A"

        if [ "$typeA" = "PROGBITS" ]; then
          ha=$(dump_hex_stream "$FILEA" "$secname")
          ho=$(dump_hex_stream "$OUTFILE" "$secname")
          [ "$ho" = "$ha" ] || fail "$secname: contenu != A"
        fi
        pass "$secname (uniquement A): OK"
        continue
      fi

      # CAS 3: Uniquement B
      if [ "$inA" -eq 0 ] && [ "$inB" -eq 1 ]; then
        sizeB_hex=$(get_size_hex "$SECB" "$secname")
        sizeB_dec=$(hex_to_dec "$sizeB_hex")
        out_dec=$(hex_to_dec "$out_size_hex")
        [ "$out_dec" -eq "$sizeB_dec" ] || fail "$secname: taille != B"

        if [ "$typeB" = "PROGBITS" ]; then
          hb=$(dump_hex_stream "$FILEB" "$secname")
          ho=$(dump_hex_stream "$OUTFILE" "$secname")
          [ "$ho" = "$hb" ] || fail "$secname: contenu != B"
        fi
        pass "$secname (uniquement B): OK"
        continue
      fi

      fail "$secname: état inattendu"
    done < "$SECU"

    echo
    pass "== Tous les tests étape 6 ont réussi =="
    ;;

  *)
    echo "Option inconnue: $OPTION"
    echo "Options valides: -f"
    exit 1
    ;;
esac