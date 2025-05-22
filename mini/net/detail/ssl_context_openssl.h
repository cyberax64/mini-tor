#pragma once
#include <mini/net/tcp_socket.h>
#include <mini/byte_buffer.h>
#include <mini/ptr.h>

#ifndef MINI_OS_WINDOWS

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>

namespace mini::net::detail {

class ssl_context
{
  public:
    static constexpr size_type max_record_size = 32 * 1024;

    ssl_context(
      void
      );

    ~ssl_context(
      void
      );

    int
    initialize(
      io::stream& sock,
      const string_ref target_name = string_ref()
      );

    void
    destroy(
      void
      );

    bool
    is_valid(
      void
      ) const;

    int
    handshake(
      void
      );

    int
    disconnect(
      void
      );

    size_type
    write(
      const byte_buffer_ref buffer
      );

    size_type
    read(
      mutable_byte_buffer_ref buffer
      );

    size_type
    get_max_message_size(
      void
      );

    size_type
    get_max_total_size(
      void
      );

  private:
    static bool _openssl_initialized;
    static void _init_openssl();

    SSL_CTX* _ssl_ctx;
    SSL* _ssl;
    io::stream* _socket;
    string _target_name;
    bool _closed;
    
    // Buffer pour la lecture
    byte_type* const _payload_recv;
    size_type _payload_recv_size;
};

}

#endif // !MINI_OS_WINDOWS