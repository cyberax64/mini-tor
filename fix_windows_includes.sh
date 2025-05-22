#!/bin/bash

# Script pour ajouter des gardes conditionnels autour des inclusions de windows.h

# Trouver tous les fichiers qui incluent windows.h
FILES=$(grep -l "#include <windows.h>" --include="*.h" --include="*.cpp" -r ./mini)

for FILE in $FILES; do
  echo "Processing $FILE"
  
  # Vérifier si le fichier contient déjà MINI_OS_WINDOWS
  if grep -q "MINI_OS_WINDOWS" "$FILE"; then
    echo "  Already has MINI_OS_WINDOWS guards, skipping"
    continue
  fi
  
  # Ajouter l'inclusion de win32_compat.h et les gardes conditionnels
  sed -i '/#include <windows.h>/i #include <mini/win32_compat.h>\n\n#ifdef MINI_OS_WINDOWS' "$FILE"
  
  # Trouver la dernière inclusion Windows et ajouter #endif après
  LAST_WINDOWS_INCLUDE=$(grep -n "#include <win" "$FILE" | tail -1 | cut -d: -f1)
  if [ ! -z "$LAST_WINDOWS_INCLUDE" ]; then
    sed -i "${LAST_WINDOWS_INCLUDE}a #endif" "$FILE"
  fi
  
  echo "  Updated $FILE"
done

echo "Done processing files"