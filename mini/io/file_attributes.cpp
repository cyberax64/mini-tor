#include "file_attributes.h"

namespace mini::io {

const file_attributes file_attributes::invalid;

file_attributes::file_attributes(
  void
  )
#ifdef MINI_OS_WINDOWS
  : _attributes(INVALID_FILE_ATTRIBUTES)
#else
  : _attributes(0xFFFFFFFF) // Valeur invalide pour Linux
#endif
{

}

file_attributes::file_attributes(
  uint32_t attributes
  )
  : _attributes(attributes)
{

}

bool
file_attributes::is_hidden(
  void
  ) const
{
#ifdef MINI_OS_WINDOWS
  return !!(_attributes & FILE_ATTRIBUTE_HIDDEN);
#else
  // Sur Linux, les fichiers cachés commencent par un point
  // Cette information n'est pas stockée dans les attributs
  return false;
#endif
}

bool
file_attributes::is_system(
  void
  ) const
{
#ifdef MINI_OS_WINDOWS
  return !!(_attributes & FILE_ATTRIBUTE_SYSTEM);
#else
  // Pas d'équivalent direct sur Linux
  return false;
#endif
}

bool
file_attributes::is_file(
  void
  ) const
{
#ifdef MINI_OS_WINDOWS
  return !!(_attributes & FILE_ATTRIBUTE_NORMAL) &&
    !(_attributes & FILE_ATTRIBUTE_DIRECTORY);
#else
  // Sur Linux, nous utilisons nos propres attributs définis
  return !(_attributes & FILE_ATTRIBUTE_DIRECTORY);
#endif
}

bool
file_attributes::is_directory(
  void
  ) const
{
#ifdef MINI_OS_WINDOWS
  return !!(_attributes & FILE_ATTRIBUTE_DIRECTORY);
#else
  return !!(_attributes & FILE_ATTRIBUTE_DIRECTORY);
#endif
}

bool
file_attributes::operator!=(
  const file_attributes& other
  ) const
{
  return !operator==(other);
}

bool
file_attributes::operator==(
  const file_attributes& other
  ) const
{
  return _attributes == other._attributes;
}

}