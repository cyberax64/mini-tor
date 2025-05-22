cmake_minimum_required(VERSION 3.10)
project(mini-tor)

# Options
option(MINI_CONFIG_DEBUG "Enable debug mode" OFF)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Définir les options de compilation
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -fpermissive -fno-strict-aliasing -Wno-unused-parameter -Wno-deprecated-declarations -D_GLIBCXX_USE_CXX11_ABI=0)
    
    # Add debug flags if in debug mode
    if(MINI_CONFIG_DEBUG)
        add_compile_options(-g -O0)
        add_definitions(-DMINI_CONFIG_DEBUG)
    else()
        add_compile_options(-O2)
    endif()
elseif(MSVC)
    add_compile_options(/W4)
    
    # Add debug flags if in debug mode
    if(MINI_CONFIG_DEBUG)
        add_compile_options(/Od /Zi)
        add_definitions(-DMINI_CONFIG_DEBUG)
    else()
        add_compile_options(/O2)
    endif()
endif()

# Add platform-specific definitions
if(UNIX AND NOT APPLE)
    add_definitions(-DLINUX -DMINI_OS_LINUX)
elseif(APPLE)
    add_definitions(-DMACOS -DMINI_OS_MACOS)
elseif(WIN32)
    add_definitions(-DWINDOWS -DMINI_OS_WINDOWS)
endif()

# Trouver les bibliothèques requises
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Inclure les répertoires d'en-têtes
include_directories(${CMAKE_SOURCE_DIR})

# Collecter tous les fichiers source
file(GLOB_RECURSE SOURCES 
    "mini/*.cpp"
    "mini/tor/*.cpp"
    "mini/tor/crypto/*.cpp"
    "mini/tor/parsers/*.cpp"
    "mini/crypto/*.cpp"
    "mini/io/*.cpp"
    "mini/net/*.cpp"
    "mini/threading/*.cpp"
    "mini/collections/*.cpp"
    "mini/crt/*.cpp"
    "mini/ctl/*.cpp"
)

# Exclure les fichiers spécifiques à Windows sous Linux
if(UNIX)
    list(FILTER SOURCES EXCLUDE REGEX "mini/win32/.*\\.cpp$")
    list(FILTER SOURCES EXCLUDE REGEX "mini/io/file_system_win32\\.cpp$")
endif()

# Ajouter l'exécutable
add_executable(mini-tor main.cpp ${SOURCES})

# Lier les bibliothèques
target_link_libraries(mini-tor 
    OpenSSL::SSL 
    OpenSSL::Crypto
    ${CMAKE_THREAD_LIBS_INIT}
)

# Ajouter des bibliothèques spécifiques à la plateforme
if(UNIX)
    # Trouver OpenSSL
    find_package(OpenSSL REQUIRED)
    include_directories(${OPENSSL_INCLUDE_DIR})
    
    target_link_libraries(mini-tor dl pthread ${OPENSSL_LIBRARIES})
endif()