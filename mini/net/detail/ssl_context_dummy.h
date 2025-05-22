#pragma once
#include <mini/net/tcp_socket.h>
#include <mini/byte_buffer.h>
#include <mini/ptr.h>

#ifndef MINI_OS_WINDOWS

namespace mini::net::detail {

class ssl_context
{
  public:
    static constexpr size_type max_record_size = 32 * 1024;

    ssl_context(
      void
      )
    {
      // Dummy implementation
    }

    ~ssl_context(
      void
      )
    {
      // Dummy implementation
    }

    void
    initialize(
      io::stream& underlying_stream,
      const string_ref target_name
      )
    {
      // Dummy implementation
      MINI_UNREFERENCED(underlying_stream);
      MINI_UNREFERENCED(target_name);
    }

    void
    disconnect(
      void
      )
    {
      // Dummy implementation
    }

    int
    handshake(
      void
      )
    {
      // Dummy implementation
      return 0;
    }

    bool
    is_valid(
      void
      ) const
    {
      // Dummy implementation
      return true;
    }

    size_type
    read(
      mutable_byte_buffer_ref buffer
      )
    {
      // Dummy implementation
      MINI_UNREFERENCED(buffer);
      return 0;
    }

    size_type
    write(
      byte_buffer_ref buffer
      )
    {
      // Dummy implementation
      MINI_UNREFERENCED(buffer);
      return 0;
    }
};

}

#endif