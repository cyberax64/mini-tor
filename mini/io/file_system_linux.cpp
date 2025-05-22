#include "file_system.h"
#include "file_attributes.h"

#ifdef MINI_OS_LINUX

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cerrno>

namespace mini::io {

file_attributes
file_system::get_attributes(
  const string_ref path
  )
{
  struct stat file_stat;
  if (stat(path.get_buffer(), &file_stat) == 0) {
    uint32_t attributes = 0;
    
    if (S_ISDIR(file_stat.st_mode)) {
      attributes |= FILE_ATTRIBUTE_DIRECTORY;
    }
    
    if (S_ISREG(file_stat.st_mode)) {
      attributes |= FILE_ATTRIBUTE_NORMAL;
    }
    
    if (!(file_stat.st_mode & S_IWUSR)) {
      attributes |= FILE_ATTRIBUTE_READONLY;
    }
    
    return file_attributes(attributes);
  }
  
  return file_attributes::invalid;
}

bool
file_system::exists(
  const string_ref path
  )
{
  return access(path.get_buffer(), F_OK) == 0;
}

bool
file_system::create_directory(
  const string_ref path
  )
{
  return mkdir(path.get_buffer(), 0755) == 0;
}

bool
file_system::remove_directory(
  const string_ref path
  )
{
  return rmdir(path.get_buffer()) == 0;
}

bool
file_system::remove_file(
  const string_ref path
  )
{
  return unlink(path.get_buffer()) == 0;
}

bool
file_system::rename(
  const string_ref old_path,
  const string_ref new_path
  )
{
  return ::rename(old_path.get_buffer(), new_path.get_buffer()) == 0;
}

}

#endif // MINI_OS_LINUX