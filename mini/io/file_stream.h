#pragma once
#include "stream.h"
#include <mini/string.h>

#include <mini/common.h>

#ifdef MINI_OS_WINDOWS
#include <windows.h>
#else
// Définitions des types équivalents pour Linux
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef void* HANDLE;
typedef unsigned long DWORD;
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
#define FILE_BEGIN SEEK_SET
#define FILE_CURRENT SEEK_CUR
#define FILE_END SEEK_END

// Définitions Windows pour les accès fichiers
#define GENERIC_READ 0x80000000L
#define GENERIC_WRITE 0x40000000L
#define CREATE_ALWAYS 2
#define CREATE_NEW 1
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define TRUNCATE_EXISTING 5
#define FILE_SHARE_DELETE 0x00000004
#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define FILE_FLAG_DELETE_ON_CLOSE 0x04000000
#define FILE_FLAG_RANDOM_ACCESS 0x10000000
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000
#define FILE_FLAG_WRITE_THROUGH 0x80000000
#endif

namespace mini::io {

enum class file_access : DWORD
{
  read            = GENERIC_READ,
  write           = GENERIC_WRITE,
  read_write      = GENERIC_READ | GENERIC_WRITE,
};

enum class file_mode : DWORD
{
  append,
  create          = CREATE_ALWAYS,
  create_new      = CREATE_NEW,
  open            = OPEN_EXISTING,
  open_or_create  = OPEN_ALWAYS,
  truncate        = TRUNCATE_EXISTING,
};

enum class file_share : DWORD
{
  none            = 0,
  dispose         = FILE_SHARE_DELETE,
  read            = FILE_SHARE_READ,
  read_write      = FILE_SHARE_READ | FILE_SHARE_WRITE,
  write           = FILE_SHARE_WRITE,
};

enum class file_options : DWORD
{
  none            = 0,
  delete_on_close = FILE_FLAG_DELETE_ON_CLOSE,
  random_access   = FILE_FLAG_RANDOM_ACCESS,
  sequential_scan = FILE_FLAG_SEQUENTIAL_SCAN,
  write_through   = FILE_FLAG_WRITE_THROUGH,
};

class file_stream
  : public stream
{
  public:
    file_stream(
      void
      )
      : _file_handle(INVALID_HANDLE_VALUE)
      , _access((file_access)0)
      , _mode((file_mode)0)
      , _share((file_share)0)
      , _options((file_options)0)
    {

    }

    file_stream(
      const string_ref path,
      file_access access = file_access::read_write,
      file_mode mode = file_mode::open_or_create,
      file_share share = file_share::read,
      file_options options = file_options::none
      )
    {
      open(path,
        access,
        mode,
        share,
        options);
    }

    ~file_stream(
      void
      )
    {
      close();
    }

    void
    open(
      const string_ref path,
      file_access access = file_access::read_write,
      file_mode mode = file_mode::open_or_create,
      file_share share = file_share::none,
      file_options options = file_options::none
      )
    {
      _path    = path;
      _access  = access;
      _mode    = mode;
      _share   = share;
      _options = options;

      DWORD real_mode = (DWORD)_mode;

      //
      // do not create new file when we're accessing
      // it only for read.
      //
      if (_access == file_access::read &&
          _mode   == file_mode::open_or_create)
      {
        _mode = file_mode::open;
        real_mode = (DWORD)_mode;
      }

      //
      // append is realy just open_or_create + seek at the end
      //
      if (_mode == file_mode::append)
      {
        real_mode = (DWORD)file_mode::open_or_create;
      }

#ifdef MINI_OS_WINDOWS
      _file_handle = CreateFile(
        path.get_buffer(),
        (DWORD)_access,
        (DWORD)_share,
        NULL,
        (DWORD)real_mode,
        (DWORD)_options,
        NULL);
#else
      // Conversion des flags Windows en flags Linux
      int flags = 0;
      
      // Access mode
      if (_access == file_access::read)
        flags |= O_RDONLY;
      else if (_access == file_access::write)
        flags |= O_WRONLY;
      else if (_access == file_access::read_write)
        flags |= O_RDWR;
        
      // File mode
      if (_mode == file_mode::create || _mode == file_mode::create_new)
        flags |= O_CREAT | O_TRUNC;
      else if (_mode == file_mode::open_or_create)
        flags |= O_CREAT;
      else if (_mode == file_mode::truncate)
        flags |= O_TRUNC;
        
      // Options
      if ((DWORD)_options & FILE_FLAG_WRITE_THROUGH)
        flags |= O_SYNC;
        
      int fd = ::open(path.get_buffer(), flags, 0666);
      _file_handle = fd < 0 ? INVALID_HANDLE_VALUE : (HANDLE)(intptr_t)fd;
#endif

      if (_mode == file_mode::append)
      {
        seek(0, stream::end);
      }
    }

    void
    close(
      void
      ) override
    {
      if (_file_handle != INVALID_HANDLE_VALUE)
      {
        //         _access  = (file_access)0;
        //         _mode    = (file_mode)0;
        //         _share   = (file_share)0;
        //         _options = (file_options)0;

#ifdef MINI_OS_WINDOWS
        CloseHandle(_file_handle);
#else
        ::close((int)(intptr_t)_file_handle);
#endif

        _file_handle = INVALID_HANDLE_VALUE;
      }
    }

    bool
    can_read(
      void
      ) const override
    {
      return ((DWORD)_access & (DWORD)file_access::read) != 0;
    }

    bool
    can_write(
      void
      ) const override
    {
      return ((DWORD)_access & (DWORD)file_access::write) != 0;
    }

    bool
    can_seek(
      void
      ) const override
    {
      return true;
    }

    size_type
    seek(
      intptr_t offset,
      seek_origin origin = seek_origin::current
      ) override
    {
      DWORD move_method = origin == seek_origin::begin
         ? FILE_BEGIN   : origin == seek_origin::current
         ? FILE_CURRENT : origin == seek_origin::end
         ? FILE_END     : FILE_CURRENT;

#ifdef MINI_OS_WINDOWS
      return SetFilePointer(_file_handle,
        (DWORD)offset,
        NULL,
        move_method);
#else
      return lseek((int)(intptr_t)_file_handle, offset, move_method);
#endif
    }

    void
    flush(
      void
      ) override
    {
#ifdef MINI_OS_WINDOWS
      FlushFileBuffers(_file_handle);
#else
      fsync((int)(intptr_t)_file_handle);
#endif
    }

    size_type
    get_size(
      void
      ) const override
    {
#ifdef MINI_OS_WINDOWS
      return GetFileSize(
        _file_handle,
        NULL);
#else
      struct stat st;
      fstat((int)(intptr_t)_file_handle, &st);
      return st.st_size;
#endif
    }

    size_type
    get_position(
      void
      ) const override
    {
      return const_cast<file_stream*>(this)->seek(0, seek_origin::current);
    }

    bool
    is_open(
      void
      ) const
    {
      return _file_handle != INVALID_HANDLE_VALUE;
    }

  private:
    size_type
    read_impl(
      void* buffer,
      size_type size
      ) override
    {
#ifdef MINI_OS_WINDOWS
      DWORD bytes_read;
      ReadFile(
        _file_handle,
        buffer,
        (DWORD)size,
        &bytes_read,
        NULL);

      return bytes_read;
#else
      return ::read((int)(intptr_t)_file_handle, buffer, size);
#endif
    }

    size_type
    write_impl(
      const void* buffer,
      size_type size
      ) override
    {
#ifdef MINI_OS_WINDOWS
      DWORD bytes_written;
      WriteFile(_file_handle,
        buffer,
        (DWORD)size,
        &bytes_written,
        NULL);

      return bytes_written;
#else
      return ::write((int)(intptr_t)_file_handle, buffer, size);
#endif
    }

    HANDLE _file_handle;
    string _path;
    file_access _access;
    file_mode _mode;
    file_share _share;
    file_options _options;
};

}
