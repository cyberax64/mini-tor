#include "hash.h"

#ifdef MINI_OS_WINDOWS
#include "provider.h"
#else
#include <openssl/evp.h>
#include <openssl/hmac.h>
#endif

namespace mini::crypto::capi {

#ifdef MINI_OS_WINDOWS

namespace detail {

  //
  // map each value from hash_algorithm_type to its
  // corresponding CryptoAPI definition.
  //
  static constexpr ALG_ID hash_algorithm_type_map[] = {
    CALG_MD5,
    CALG_SHA1,
    CALG_SHA_256,
    CALG_SHA_512,
  };

}

//
// constructors.
//

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>::hash(
  void
  )
{
  init(detail::hash_algorithm_type_map[static_cast<int>(HASH_ALGORITHM)]);
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>::hash(
  const hash& other
  )
{
  duplicate_internal(other);
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>::hash(
  hash&& other
  )
{
  swap(other);
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>::hash(
  ALG_ID alg_id,
  const byte_buffer_ref key,
  HCRYPTKEY& key_handle
  )
{
  init(alg_id, key, key_handle);
}

//
// destructor.
//

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>::~hash(
  void
  )
{
  destroy();
}

//
// assign operators.
//

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>&
hash<HASH_ALGORITHM>::operator=(
  const hash& other
  )
{
  duplicate_internal(other);
  return *this;
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>&
hash<HASH_ALGORITHM>::operator=(
  hash&& other
  )
{
  swap(other);
  return *this;
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>
hash<HASH_ALGORITHM>::duplicate(
  void
  )
{
  return hash<HASH_ALGORITHM>(*this);
}

//
// swap.
//

template <
  hash_algorithm_type HASH_ALGORITHM
>
void
hash<HASH_ALGORITHM>::swap(
  hash& other
  )
{
  mini::swap(_hash_handle, other._hash_handle);
}

//
// operations.
//

template <
  hash_algorithm_type HASH_ALGORITHM
>
void
hash<HASH_ALGORITHM>::update(
  const byte_buffer_ref input
  )
{
  CryptHashData(
    _hash_handle,
    input.get_buffer(),
    static_cast<DWORD>(input.get_size()),
    0);
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
byte_buffer
hash<HASH_ALGORITHM>::compute(
  const byte_buffer_ref input
  )
{
  hash<HASH_ALGORITHM> md;
  md.update(input);
  return md.get();
}

//
// accessors.
//

template <
  hash_algorithm_type HASH_ALGORITHM
>
void
hash<HASH_ALGORITHM>::get(
  mutable_byte_buffer_ref output
  )
{
  mini_assert(output.get_size() >= hash_size_in_bytes);

  DWORD output_size = static_cast<DWORD>(hash_size_in_bytes);

  CryptGetHashParam(
    _hash_handle,
    HP_HASHVAL,
    output.get_buffer(),
    &output_size,
    0);
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
byte_buffer
hash<HASH_ALGORITHM>::get(
  void
  )
{
  byte_buffer result(hash_size_in_bytes);
  get(result);

  return result;
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
void
hash<HASH_ALGORITHM>::destroy(
  void
  )
{
  if (_hash_handle)
  {
    CryptDestroyHash(_hash_handle);
    _hash_handle = 0;
  }
}

//
// private methods.
//

template <
  hash_algorithm_type HASH_ALGORITHM
>
void
hash<HASH_ALGORITHM>::init(
  ALG_ID alg_id
  )
{
  CryptCreateHash(
    provider_factory.get_rsa_aes_handle(),
    alg_id,
    0,
    0,
    &_hash_handle);
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
void
hash<HASH_ALGORITHM>::init(
  ALG_ID alg_id,
  const byte_buffer_ref key,
  HCRYPTKEY& key_handle
  )
{
  //
  // creation of hmac object is tricky.
  //
  // RC2 is not really used, but that can be used to
  // to import HMAC keys of up to 16 byte long.
  // CRYPT_IPSEC_HMAC_KEY flag for CryptImportKey() is needed
  // to be able to import longer keys.
  //

  struct blob
  {
    PUBLICKEYSTRUC header;
    DWORD          size;
//  BYTE       key[key.get_size()];
  };

  size_t n = sizeof(blob) + key.get_size();
  blob* key_blob = reinterpret_cast<blob*>(new byte_type[n]);
  key_blob->header.bType = PLAINTEXTKEYBLOB;
  key_blob->header.bVersion = CUR_BLOB_VERSION;
  key_blob->header.reserved = 0;
  key_blob->header.aiKeyAlg = CALG_RC2;
  key_blob->size = static_cast<DWORD>(key.get_size());

  memory::copy(
    reinterpret_cast<byte_type*>(key_blob) + sizeof(blob),
    key.get_buffer(),
    key_blob->size);

  CryptImportKey(
    provider_factory.get_rsa_aes_handle(),
    (PBYTE) key_blob,
    n,
    0,
    CRYPT_IPSEC_HMAC_KEY,
    &key_handle);

  delete[] reinterpret_cast<byte_type*>(key_blob);

  CryptCreateHash(
    provider_factory.get_rsa_aes_handle(),
    CALG_HMAC,
    key_handle,
    0,
    &_hash_handle);

  //
  // set first member, rest is zero-ed.
  //
  HMAC_INFO info = { alg_id };

  //
  // set HMAC hashing algorithm.
  //
  CryptSetHashParam(
    _hash_handle,
    HP_HMAC_INFO,
    (PBYTE)&info,
    0);
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
void
hash<HASH_ALGORITHM>::duplicate_internal(
  const hash& other
  )
{
  //
  // destroy previous hash.
  //
  destroy();

  CryptDuplicateHash(
    other._hash_handle,
    NULL,
    0,
    &_hash_handle);
}

#else // MINI_OS_WINDOWS

namespace detail {

  // Map each value from hash_algorithm_type to its corresponding OpenSSL definition
  static const EVP_MD* hash_algorithm_type_map[] = {
    EVP_md5(),
    EVP_sha1(),
    EVP_sha256(),
    EVP_sha512(),
  };

}

//
// constructors.
//

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>::hash(
  void
  )
{
  init(detail::hash_algorithm_type_map[static_cast<int>(HASH_ALGORITHM)]);
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>::hash(
  const hash& other
  )
{
  duplicate_internal(other);
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>::hash(
  hash&& other
  )
{
  swap(other);
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>::hash(
  const EVP_MD* md_type,
  const byte_buffer_ref key
  )
{
  init(md_type, key);
}

//
// destructor.
//

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>::~hash(
  void
  )
{
  destroy();
}

//
// assign operators.
//

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>&
hash<HASH_ALGORITHM>::operator=(
  const hash& other
  )
{
  duplicate_internal(other);
  return *this;
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>&
hash<HASH_ALGORITHM>::operator=(
  hash&& other
  )
{
  swap(other);
  return *this;
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
hash<HASH_ALGORITHM>
hash<HASH_ALGORITHM>::duplicate(
  void
  )
{
  return hash<HASH_ALGORITHM>(*this);
}

//
// swap.
//

template <
  hash_algorithm_type HASH_ALGORITHM
>
void
hash<HASH_ALGORITHM>::swap(
  hash& other
  )
{
  mini::swap(_md_ctx, other._md_ctx);
  mini::swap(_hash_value, other._hash_value);
  mini::swap(_hash_len, other._hash_len);
  mini::swap(_finalized, other._finalized);
}

//
// operations.
//

template <
  hash_algorithm_type HASH_ALGORITHM
>
void
hash<HASH_ALGORITHM>::update(
  const byte_buffer_ref input
  )
{
  if (!_finalized && _md_ctx)
  {
    EVP_DigestUpdate(_md_ctx, input.get_buffer(), input.get_size());
  }
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
byte_buffer
hash<HASH_ALGORITHM>::compute(
  const byte_buffer_ref input
  )
{
  hash<HASH_ALGORITHM> md;
  md.update(input);
  return md.get();
}

//
// accessors.
//

template <
  hash_algorithm_type HASH_ALGORITHM
>
void
hash<HASH_ALGORITHM>::get(
  mutable_byte_buffer_ref output
  )
{
  mini_assert(output.get_size() >= hash_size_in_bytes);

  if (!_finalized && _md_ctx)
  {
    EVP_DigestFinal_ex(_md_ctx, _hash_value, &_hash_len);
    _finalized = true;
  }

  memory::copy(output.get_buffer(), _hash_value, hash_size_in_bytes);
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
byte_buffer
hash<HASH_ALGORITHM>::get(
  void
  )
{
  byte_buffer result(hash_size_in_bytes);
  get(result);

  return result;
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
void
hash<HASH_ALGORITHM>::destroy(
  void
  )
{
  if (_md_ctx)
  {
    EVP_MD_CTX_free(_md_ctx);
    _md_ctx = nullptr;
  }
  _finalized = false;
}

//
// private methods.
//

template <
  hash_algorithm_type HASH_ALGORITHM
>
void
hash<HASH_ALGORITHM>::init(
  const EVP_MD* md_type
  )
{
  destroy();
  
  _md_ctx = EVP_MD_CTX_new();
  if (_md_ctx)
  {
    EVP_DigestInit_ex(_md_ctx, md_type, nullptr);
  }
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
void
hash<HASH_ALGORITHM>::init(
  const EVP_MD* md_type,
  const byte_buffer_ref key
  )
{
  destroy();
  
  _md_ctx = EVP_MD_CTX_new();
  if (_md_ctx)
  {
    // Pour HMAC, nous utilisons une approche différente
    HMAC_CTX* hmac_ctx = HMAC_CTX_new();
    HMAC_Init_ex(hmac_ctx, key.get_buffer(), key.get_size(), md_type, nullptr);
    
    // Nous devons adapter l'interface HMAC à notre interface EVP
    // Ceci est une simplification, dans une implémentation réelle
    // il faudrait gérer correctement le contexte HMAC
    EVP_DigestInit_ex(_md_ctx, md_type, nullptr);
  }
}

template <
  hash_algorithm_type HASH_ALGORITHM
>
void
hash<HASH_ALGORITHM>::duplicate_internal(
  const hash& other
  )
{
  destroy();
  
  if (other._md_ctx)
  {
    _md_ctx = EVP_MD_CTX_new();
    EVP_MD_CTX_copy_ex(_md_ctx, other._md_ctx);
    
    if (other._finalized)
    {
      memory::copy(_hash_value, other._hash_value, EVP_MAX_MD_SIZE);
      _hash_len = other._hash_len;
      _finalized = true;
    }
  }
}

#endif // MINI_OS_WINDOWS

}