#include "ssl_stream.h"

namespace mini::net {

ssl_stream::ssl_stream(
  io::stream& underlying_stream,
  const string_ref target_name
  )
  : _underlying_stream(underlying_stream)
{
#ifdef MINI_OS_WINDOWS
  _context.initialize(_underlying_stream, target_name);
#else
  _context = new detail::ssl_context();
  _context->initialize(_underlying_stream, target_name);
#endif
}

ssl_stream::~ssl_stream(
  void
  )
{
  close();
#ifndef MINI_OS_WINDOWS
  if (_context)
  {
    delete _context;
    _context = nullptr;
  }
#endif
}

void
ssl_stream::close(
  void
  )
{
#ifdef MINI_OS_WINDOWS
  _context.disconnect();
#else
  if (_context)
  {
    _context->disconnect();
  }
#endif
}

bool
ssl_stream::can_read(
  void
  ) const
{
  return true;
}

bool
ssl_stream::can_write(
  void
  ) const
{
  return true;
}

bool
ssl_stream::can_seek(
  void
  ) const
{
  return false;
}

bool
ssl_stream::handshake(
  const string_ref host,
  uint16_t port
  )
{
  MINI_UNREFERENCED(host);
  MINI_UNREFERENCED(port);

#ifdef MINI_OS_WINDOWS
  return _context.handshake() == SEC_E_OK;
#else
  return _context && _context->handshake() == 0;
#endif
}

size_type
ssl_stream::seek(
  intptr_t offset,
  seek_origin origin
  )
{
  MINI_UNREFERENCED(offset);
  MINI_UNREFERENCED(origin);

  return 0;
}

void
ssl_stream::flush()
{

}

size_type
ssl_stream::get_size(
  void
  ) const
{
  return 0;
}

size_type
ssl_stream::get_position(
  void
  ) const
{
  return 0;
}

io::stream&
ssl_stream::get_underlying_stream(
  void
  )
{
  return _underlying_stream;
}

bool
ssl_stream::is_handshake_established(
  void
  ) const
{
#ifdef MINI_OS_WINDOWS
  return _context.is_valid();
#else
  return _context && _context->is_valid();
#endif
}

size_type
ssl_stream::read_impl(
  void* buffer,
  size_type size
  )
{
  mutable_byte_buffer_ref buf(
    static_cast<byte_type*>(buffer),
    static_cast<byte_type*>(buffer) + size);

#ifdef MINI_OS_WINDOWS
  return _context.read(buf);
#else
  return _context ? _context->read(buf) : 0;
#endif
}

size_type
ssl_stream::write_impl(
  const void* buffer,
  size_type size
  )
{
  byte_buffer_ref buf(
    static_cast<const byte_type*>(buffer),
    static_cast<const byte_type*>(buffer) + size);

#ifdef MINI_OS_WINDOWS
  return _context.write(buf);
#else
  return _context ? _context->write(buf) : 0;
#endif
}

}
