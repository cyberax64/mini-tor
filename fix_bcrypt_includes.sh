#!/bin/bash

# Script pour ajouter des gardes conditionnels autour des inclusions de bcrypt.h

# Trouver tous les fichiers qui incluent bcrypt.h
FILES=$(grep -l "#include <bcrypt.h>" --include="*.h" --include="*.cpp" -r ./mini)

for FILE in $FILES; do
  echo "Processing $FILE"
  
  # Vérifier si le fichier contient déjà MINI_OS_WINDOWS
  if grep -q "MINI_OS_WINDOWS" "$FILE"; then
    # Vérifier si bcrypt.h est déjà dans un bloc conditionnel
    if grep -A1 "#ifdef MINI_OS_WINDOWS" "$FILE" | grep -q "bcrypt.h"; then
      echo "  bcrypt.h already in conditional block, skipping"
      continue
    fi
    
    # Remplacer l'inclusion directe par une inclusion conditionnelle
    sed -i 's/#include <bcrypt.h>/#ifdef MINI_OS_WINDOWS\n#include <bcrypt.h>\n#else\n\/\/ Linux\/OpenSSL equivalent headers\n#include <openssl\/rsa.h>\n#include <openssl\/pem.h>\n#endif/g' "$FILE"
  else
    # Ajouter l'inclusion de win32_compat.h et les gardes conditionnels
    sed -i '/#include <bcrypt.h>/i #include <mini/win32_compat.h>\n\n#ifdef MINI_OS_WINDOWS' "$FILE"
    sed -i 's/#include <bcrypt.h>/#include <bcrypt.h>\n#else\n\/\/ Linux\/OpenSSL equivalent headers\n#include <openssl\/rsa.h>\n#include <openssl\/pem.h>\n#endif/g' "$FILE"
  fi
  
  echo "  Updated $FILE"
done

echo "Done processing files"