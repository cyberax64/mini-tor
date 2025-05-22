from setuptools import setup, Extension
import os
import platform

# Déterminer le système d'exploitation
is_windows = platform.system() == 'Windows'
is_linux = platform.system() == 'Linux'
is_macos = platform.system() == 'Darwin'

# Chemins des sources
mini_tor_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
source_files = [
    os.path.join('python_bindings.cpp'),
]

# Inclure les fichiers sources principaux de mini-tor
for root, dirs, files in os.walk(os.path.join(mini_tor_dir, 'mini')):
    for file in files:
        if file.endswith('.cpp'):
            rel_path = os.path.relpath(os.path.join(root, file), mini_tor_dir)
            source_files.append(rel_path)

# Définir les options de compilation
extra_compile_args = []
extra_link_args = []
include_dirs = [mini_tor_dir]
library_dirs = []
libraries = []

# Options spécifiques à Windows
if is_windows:
    extra_compile_args.extend(['/std:c++17', '/EHsc', '/DWIN32', '/D_WINDOWS'])
    libraries.extend(['ws2_32', 'crypt32', 'bcrypt', 'secur32', 'shlwapi'])

# Options spécifiques à Linux
if is_linux:
    extra_compile_args.extend(['-std=c++17', '-DMINI_OS_LINUX'])
    libraries.extend(['ssl', 'crypto', 'pthread'])

# Options spécifiques à macOS
if is_macos:
    extra_compile_args.extend(['-std=c++17', '-DMINI_OS_MACOS'])
    libraries.extend(['ssl', 'crypto'])

# Définir l'extension
mini_tor_module = Extension(
    'mini_tor',
    sources=source_files,
    include_dirs=include_dirs,
    library_dirs=library_dirs,
    libraries=libraries,
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
    language='c++'
)

setup(
    name='mini_tor',
    version='0.1.0',
    description='Python bindings for mini-tor library',
    author='OpenHands',
    author_email='openhands@all-hands.dev',
    ext_modules=[mini_tor_module],
    py_modules=['mini_tor_py'],
)