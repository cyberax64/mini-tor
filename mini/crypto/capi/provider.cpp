#include "provider.h"

namespace mini::crypto::capi {

provider provider_factory;

provider::provider(
  void
  )
{
  init();
}

provider::~provider(
  void
  )
{
  destroy();
}

HCRYPTPROV
provider::get_rsa_aes_handle(
  void
  )
{
  return _provider_rsa_aes_handle;
}

HCRYPTPROV
provider::get_dh_handle(
  void
  )
{
  return _provider_dh_handle;
}

void
provider::init(
  void
  )
{
#ifdef MINI_OS_WINDOWS
  //
  // alternative provider
  //
  if (GetEnvironmentVariable(
    TEXT("csp_alt_prov"),
    nullptr,
    0))
  {
    LoadLibrary(TEXT("csp_alt.dll"));
  }

  CryptAcquireContext(
    &_provider_rsa_aes_handle,
    NULL,
    MS_ENH_RSA_AES_PROV,
    PROV_RSA_AES,
    CRYPT_VERIFYCONTEXT);

  CryptAcquireContext(
    &_provider_dh_handle,
    NULL,
    MS_ENH_DSS_DH_PROV,
    PROV_DSS_DH,
    CRYPT_VERIFYCONTEXT);
#else
  // Sur Linux, nous utilisons OpenSSL directement, donc pas besoin d'initialiser des handles
  _provider_rsa_aes_handle = nullptr;
  _provider_dh_handle = nullptr;
#endif
}

void
provider::destroy(
  void
  )
{
#ifdef MINI_OS_WINDOWS
  if (_provider_dh_handle)
  {
    CryptReleaseContext(
      _provider_dh_handle,
      0);
    _provider_dh_handle = 0;
  }

  if (_provider_rsa_aes_handle)
  {
    CryptReleaseContext(
      _provider_rsa_aes_handle,
      0);
    _provider_rsa_aes_handle = 0;
  }
#else
  // Rien à faire sur Linux
  _provider_dh_handle = nullptr;
  _provider_rsa_aes_handle = nullptr;
#endif
}

}
