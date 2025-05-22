#!/bin/bash

# Corriger les problèmes de compilation pour Linux

# Ajouter -D_GLIBCXX_USE_CXX11_ABI=0 pour la compatibilité avec les anciennes bibliothèques
sed -i 's/add_compile_options(-Wall -Wextra -fpermissive -fno-strict-aliasing -Wno-unused-parameter -Wno-deprecated-declarations)/add_compile_options(-Wall -Wextra -fpermissive -fno-strict-aliasing -Wno-unused-parameter -Wno-deprecated-declarations -D_GLIBCXX_USE_CXX11_ABI=0)/' CMakeLists.txt

# Corriger le problème de __cdecl dans main.cpp
sed -i 's/int __cdecl/int/' main.cpp

# Corriger le problème de template dans buffer_ref.h
sed -i 's/template<\s*typename T\s*>/template<typename T, typename = void>/' mini/buffer_ref.h

# Corriger le problème de hash.h
sed -i 's/: std::hash<T>/: std::hash<T> {/' mini/hash.h
sed -i '/: std::hash<T> {/a\};' mini/hash.h

# Corriger le problème de traits_type dans string_ref.inl
sed -i 's/traits_type::length(value)/strlen(value)/' mini/string_ref.inl

# Ajouter les includes manquants
echo "#include <cstring>" > /tmp/includes.txt
cat mini/string_ref.inl | cat /tmp/includes.txt - > /tmp/string_ref.inl
mv /tmp/string_ref.inl mini/string_ref.inl

# Corriger les problèmes de redéfinition de types
find mini -name "*.h" -exec sed -i 's/using size_type\s*=\s*size_type;/using size_type = ::mini::size_type;/g' {} \;
find mini -name "*.h" -exec sed -i 's/using pointer_difference_type\s*=\s*pointer_difference_type;/using pointer_difference_type = ::mini::pointer_difference_type;/g' {} \;

echo "Corrections appliquées. Essayez de compiler à nouveau."