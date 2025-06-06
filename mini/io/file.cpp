#include "file.h"

#include <mini/io/file_stream.h>

#ifdef MINI_OS_WINDOWS
#include <shlwapi.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace mini::io {

file_attributes
file::get_attributes(
  const string_ref path
  )
{
#ifdef MINI_OS_WINDOWS
  return file_attributes(GetFileAttributes(path.get_buffer()));
#else
  struct stat st;
  if (stat(path.get_buffer(), &st) != 0) {
    return file_attributes(0xFFFFFFFF); // INVALID_FILE_ATTRIBUTES
  }
  
  DWORD attr = 0;
  if (S_ISDIR(st.st_mode)) {
    attr |= FILE_ATTRIBUTE_DIRECTORY;
  }
  if (!(st.st_mode & S_IWUSR)) {
    attr |= FILE_ATTRIBUTE_READONLY;
  }
  if (S_ISLNK(st.st_mode)) {
    attr |= FILE_ATTRIBUTE_REPARSE_POINT;
  }
  
  return file_attributes(attr);
#endif
}

bool
file::exists(
  const string_ref path
  )
{
  auto attributes = get_attributes(path.get_buffer());

  return
     attributes != file_attributes::invalid &&
    !attributes.is_directory();
}

string
file::read_to_string(
  const string_ref path
  )
{
  io::file_stream f(path, io::file_access::read);

  string result;
  result.resize(f.get_size());
  f.read(&result[0], f.get_size());

  return result;
}

collections::list<string>
file::read_all_lines(
  const string_ref path
  )
{
  return read_to_string(path).split("\n");
}

void
file::write_from_string(
  const string_ref path,
  const byte_buffer_ref content
  )
{
  io::file_stream f(path, io::file_access::write);
  f.write(content.get_buffer(), content.get_size());
}

}
