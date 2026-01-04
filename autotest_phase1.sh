#!/bin/sh

OPTION="$1"
FILE="$2"
SECTION=""

# Gestion spéciale pour -x (3 arguments)
if [ "$OPTION" = "-x" ]; then
    SECTION="$2"
    FILE="$3"
fi

if [ -z "$OPTION" ] || [ -z "$FILE" ]; then
    echo "Usage:"
    echo "  $0 -t <fichier-elf>                    # Test en-tête ELF"
    echo "  $0 -S <fichier-elf>                    # Test sections"
    echo "  $0 -s <fichier-elf>                    # Test symboles"
    echo "  $0 -x <section> <fichier-elf>          # Test contenu section"
    echo "  $0 -r <fichier-elf>                    # Test relocations"
    exit 1
fi

MINE="/tmp/mine.txt"
READELF="/tmp/readelf.txt"

case "$OPTION" in
    -t)
        echo "== Autotest ELF HEADER =="

        ./Options_and_debug_example -t "$FILE" | tail -n 19 > "$MINE"
        arm-none-eabi-readelf -h "$FILE" | tail -n 19 > "$READELF"
        ;;
        
    -S)
        echo "== Autotest SECTION HEADERS =="

        ./Options_and_debug_example -S "$FILE" \
          | grep -E '^[[:space:]]*\[' \
          | tr -s ' ' \
          > "$MINE"

        arm-none-eabi-readelf --wide -S "$FILE" \
          | grep -E '^[[:space:]]*\[' \
          | tr -s ' ' \
          > "$READELF"
        ;;
        
    -s) 
        echo "== Autotest SYMBOL TABLE =="

        ./Options_and_debug_example -s "$FILE" \
          | grep -E '^[[:space:]]*[0-9]+' \
          | tr -s ' ' \
          > "$MINE"

        arm-none-eabi-readelf --wide -s "$FILE" \
          | grep -E '^[[:space:]]*[0-9]+' \
          | tr -s ' ' \
          > "$READELF"
        ;;
        
    -x)
        echo "== Autotest SECTION CONTENT ($SECTION) =="

        ./Options_and_debug_example -x "$SECTION" "$FILE" \
          | grep -E '^\s+0x[0-9a-f]+' \
          | tr -s ' ' \
          | sed 's/\s\+$//' \
          > "$MINE"

        arm-none-eabi-readelf -x "$SECTION" "$FILE" \
          | grep -E '^\s+0x[0-9a-f]+' \
          | tr -s ' ' \
          | sed 's/ \.\.\..*//' \
          | sed 's/\s\+$//' \
          > "$READELF"
        ;;
    
    -r)
        echo "== Autotest RELOCATIONS =="

        ./Options_and_debug_example -r "$FILE" \
          | grep -E '^\s+[0-9a-f]{8}' \
          | tr -s ' ' \
          | awk '{print $1, $2, $3}' \
          | sed 's/\s\+$//' \
          > "$MINE"

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
          > "$READELF"
        ;;
        
    *)
        echo "Option inconnue: $OPTION"
        echo "Options valides: -t, -S, -s, -x, -r"
        exit 1
        ;;
esac

echo "===== DIFF ====="
diff -u -w "$READELF" "$MINE"

if [ $? -eq 0 ]; then
    echo "✅ Test réussi !"
else
    echo "❌ Des différences détectées"
fi