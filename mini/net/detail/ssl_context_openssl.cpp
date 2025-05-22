#include "ssl_context_openssl.h"

#ifndef MINI_OS_WINDOWS

#include <mini/logger.h>

namespace mini::net::detail {

bool ssl_context::_openssl_initialized = false;

void ssl_context::_init_openssl()
{
  if (!_openssl_initialized)
  {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    _openssl_initialized = true;
  }
}

ssl_context::ssl_context(
  void
  )
  : _ssl_ctx(nullptr)
  , _ssl(nullptr)
  , _socket(nullptr)
  , _closed(true)
  , _payload_recv(new byte_type[max_record_size])
  , _payload_recv_size(0)
{
  _init_openssl();
}

ssl_context::~ssl_context(
  void
  )
{
  destroy();
  delete[] _payload_recv;
}

int
ssl_context::initialize(
  io::stream& sock,
  const string_ref target_name
  )
{
  _socket = &sock;
  _target_name = target_name;
  _closed = false;

  // Créer un contexte SSL
  _ssl_ctx = SSL_CTX_new(SSLv23_client_method());
  if (!_ssl_ctx)
  {
    mini_warning("SSL_CTX_new failed");
    return -1;
  }

  // Configurer les options SSL
  SSL_CTX_set_options(_ssl_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
  
  // Créer une structure SSL
  _ssl = SSL_new(_ssl_ctx);
  if (!_ssl)
  {
    mini_warning("SSL_new failed");
    SSL_CTX_free(_ssl_ctx);
    _ssl_ctx = nullptr;
    return -1;
  }

  // Configurer le mode BIO personnalisé
  BIO* bio = BIO_new(BIO_s_mem());
  if (!bio)
  {
    mini_warning("BIO_new failed");
    SSL_free(_ssl);
    _ssl = nullptr;
    SSL_CTX_free(_ssl_ctx);
    _ssl_ctx = nullptr;
    return -1;
  }

  SSL_set_bio(_ssl, bio, bio);
  
  return 0;
}

void
ssl_context::destroy(
  void
  )
{
  if (_ssl)
  {
    SSL_free(_ssl);
    _ssl = nullptr;
  }

  if (_ssl_ctx)
  {
    SSL_CTX_free(_ssl_ctx);
    _ssl_ctx = nullptr;
  }

  _closed = true;
}

bool
ssl_context::is_valid(
  void
  ) const
{
  return _ssl != nullptr && _ssl_ctx != nullptr && !_closed;
}

int
ssl_context::handshake(
  void
  )
{
  if (!is_valid())
  {
    return -1;
  }

  // Effectuer la poignée de main SSL
  int result = SSL_connect(_ssl);
  if (result != 1)
  {
    int ssl_error = SSL_get_error(_ssl, result);
    mini_warning("SSL_connect failed with error: %d", ssl_error);
    return -1;
  }

  // Vérifier le certificat
  if (SSL_get_verify_result(_ssl) != X509_V_OK)
  {
    mini_warning("Certificate verification failed");
    // On continue quand même, car Tor n'utilise pas de certificats standards
  }

  return 0;
}

int
ssl_context::disconnect(
  void
  )
{
  if (!is_valid())
  {
    return -1;
  }

  int result = SSL_shutdown(_ssl);
  if (result < 0)
  {
    int ssl_error = SSL_get_error(_ssl, result);
    mini_warning("SSL_shutdown failed with error: %d", ssl_error);
    return -1;
  }

  destroy();
  return 0;
}

size_type
ssl_context::write(
  const byte_buffer_ref buffer
  )
{
  if (!is_valid() || buffer.get_size() == 0)
  {
    return 0;
  }

  int bytes_written = SSL_write(_ssl, buffer.get_buffer(), (int)buffer.get_size());
  if (bytes_written <= 0)
  {
    int ssl_error = SSL_get_error(_ssl, bytes_written);
    mini_warning("SSL_write failed with error: %d", ssl_error);
    return 0;
  }

  // Récupérer les données du BIO et les envoyer via le socket
  char bio_buffer[max_record_size];
  int pending = BIO_ctrl_pending(SSL_get_wbio(_ssl));
  if (pending > 0)
  {
    int read = BIO_read(SSL_get_wbio(_ssl), bio_buffer, pending);
    if (read > 0)
    {
      return _socket->write(bio_buffer, read);
    }
  }

  return bytes_written;
}

size_type
ssl_context::read(
  mutable_byte_buffer_ref buffer
  )
{
  if (!is_valid() || buffer.get_size() == 0)
  {
    return 0;
  }

  // Lire les données du socket et les mettre dans le BIO
  size_type bytes_read = _socket->read(_payload_recv, max_record_size);
  if (bytes_read > 0)
  {
    BIO_write(SSL_get_rbio(_ssl), _payload_recv, (int)bytes_read);
  }
  else if (bytes_read == 0)
  {
    // Connection fermée
    _closed = true;
    return 0;
  }

  // Lire les données déchiffrées
  int ssl_bytes_read = SSL_read(_ssl, buffer.get_buffer(), (int)buffer.get_size());
  if (ssl_bytes_read <= 0)
  {
    int ssl_error = SSL_get_error(_ssl, ssl_bytes_read);
    if (ssl_error == SSL_ERROR_WANT_READ)
    {
      // Besoin de plus de données
      return 0;
    }
    else
    {
      mini_warning("SSL_read failed with error: %d", ssl_error);
      _closed = true;
      return 0;
    }
  }

  return ssl_bytes_read;
}

size_type
ssl_context::get_max_message_size(
  void
  )
{
  return max_record_size;
}

size_type
ssl_context::get_max_total_size(
  void
  )
{
  return max_record_size;
}

}

#endif // !MINI_OS_WINDOWS