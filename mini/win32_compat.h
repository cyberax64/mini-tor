#pragma once

#include <mini/common.h>

#ifndef MINI_OS_WINDOWS
// Définitions des types Windows pour Linux
typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef unsigned long ULONG;
typedef const wchar_t* LPCWSTR;
typedef void* HCRYPTPROV;
typedef void* HCRYPTKEY;
typedef void* BCRYPT_ALG_HANDLE;
typedef void* BCRYPT_KEY_HANDLE;
typedef unsigned char* PUCHAR;
typedef unsigned short WORD;
typedef int BOOL;
typedef void* HCRYPTHASH;

// Définitions pour les sockets
#define SOCKET int
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

// Définitions pour la cryptographie
#define CRYPT_EXPORTABLE 0x00000001
#define KP_MODE 4
#define KP_IV 1

#define PLAINTEXTKEYBLOB 0x8
#define CUR_BLOB_VERSION 2
#define FALSE 0

// Définitions pour BCrypt
#define BCRYPT_KEY_DATA_BLOB_MAGIC 0x4d42444b
#define BCRYPT_KEY_DATA_BLOB_VERSION1 1

// Définitions pour les algorithmes AES
#define CALG_AES_128 0x660E
#define CALG_AES_192 0x660F
#define CALG_AES_256 0x6610

// Définitions pour les modes de chiffrement
#define CRYPT_MODE_CBC 1
#define CRYPT_MODE_ECB 2
#define CRYPT_MODE_OFB 3
#define CRYPT_OAEP 0x00000040

// Fonction CryptDecodeObject
inline BOOL CryptDecodeObject(
    DWORD dwCertEncodingType,
    DWORD lpszStructType,
    const BYTE* pbEncoded,
    DWORD cbEncoded,
    DWORD dwFlags,
    void* pvStructInfo,
    DWORD* pcbStructInfo) {
    // Stub pour Linux
    return FALSE;
}
#define CRYPT_MODE_CFB 4
#define CRYPT_MODE_CTS 5

// Structures pour la cryptographie
typedef struct _BLOBHEADER {
    BYTE bType;
    BYTE bVersion;
    WORD reserved;
    DWORD aiKeyAlg;
} BLOBHEADER;

typedef struct _BCRYPT_KEY_DATA_BLOB_HEADER {
    DWORD dwMagic;
    DWORD dwVersion;
    DWORD cbKeyData;
} BCRYPT_KEY_DATA_BLOB_HEADER;

// Définitions pour les algorithmes
typedef DWORD ALG_ID;

// Constantes pour les algorithmes
#define BCRYPT_KEY_DATA_BLOB (LPCWSTR)L"KeyDataBlob"
#define BCRYPT_DH_PUBLIC_BLOB (LPCWSTR)L"DHPUBLICBLOB"
#define BCRYPT_DH_PRIVATE_BLOB (LPCWSTR)L"DHPRIVATEBLOB"
#define BCRYPT_DH_PUBLIC_MAGIC 0x42504844  // DHPB
#define BCRYPT_DH_PRIVATE_MAGIC 0x56504844 // DHPV

#ifndef BCRYPT_KDF_RAW_SECRET
#define BCRYPT_KDF_RAW_SECRET (LPCWSTR)L"RAWSECRET"

// Définitions pour RSA
#define BCRYPT_RSAPUBLIC_BLOB L"RSAPUBLICBLOB"
#define BCRYPT_RSAPRIVATE_BLOB L"RSAPRIVATEBLOB"
#define BCRYPT_RSAFULLPRIVATE_BLOB L"RSAFULLPRIVATEBLOB"

#define BCRYPT_RSAPUBLIC_MAGIC 0x31415352  // RSA1
#define BCRYPT_RSAFULLPRIVATE_MAGIC 0x33415352  // RSA3

#define BCRYPT_PAD_PKCS1 0x00000002
#define BCRYPT_PAD_OAEP 0x00000004

#define BCRYPT_SHA1_ALGORITHM L"SHA1"

// Structure pour RSA
typedef struct _BCRYPT_RSAKEY_BLOB {
    ULONG Magic;
    ULONG BitLength;
    ULONG cbPublicExp;
    ULONG cbModulus;
    ULONG cbPrime1;
    ULONG cbPrime2;
} BCRYPT_RSAKEY_BLOB;

typedef struct _BCRYPT_OAEP_PADDING_INFO {
    LPCWSTR pszAlgId;
    PUCHAR pbLabel;
    ULONG cbLabel;
} BCRYPT_OAEP_PADDING_INFO;

// Définitions pour CryptDecodeObject
#define X509_ASN_ENCODING 0x00000001
#define PKCS_7_ASN_ENCODING 0x00010000
#define RSA_CSP_PUBLICKEYBLOB 19
#define PKCS_RSA_PRIVATE_KEY 43

// Structure pour RSAPUBKEY
typedef struct _RSAPUBKEY {
    DWORD magic;
    DWORD bitlen;
    DWORD pubexp;
} RSAPUBKEY;
#endif

// Définitions pour DH
#define CALG_DH_EPHEM 0x0000AA02
#define PRIVATEKEYBLOB 0x7
#define PUBLICKEYBLOB 0x6

// Structures pour DH
typedef struct _BCRYPT_DH_KEY_BLOB {
    DWORD dwMagic;
    DWORD cbKey;
} BCRYPT_DH_KEY_BLOB;

typedef struct _DHPUBKEY {
    DWORD magic;
    DWORD bitlen;
} DHPUBKEY;

typedef struct _PUBLICKEYSTRUC {
    BYTE bType;
    BYTE bVersion;
    WORD reserved;
    DWORD aiKeyAlg;
} PUBLICKEYSTRUC;

// Définition du type NTSTATUS
typedef long NTSTATUS;

// Fonctions BCrypt pour DH
typedef void* BCRYPT_SECRET_HANDLE;

// Fonction BCryptSecretAgreement
inline NTSTATUS BCryptSecretAgreement(
    BCRYPT_KEY_HANDLE hPrivKey,
    BCRYPT_KEY_HANDLE hPubKey,
    BCRYPT_SECRET_HANDLE *phAgreedSecret,
    ULONG dwFlags) {
    // Stub pour Linux
    return 0;
}

inline NTSTATUS BCryptDeriveKey(
    BCRYPT_SECRET_HANDLE hSharedSecret,
    LPCWSTR pwszKDF,
    void* pParameterList,
    PUCHAR pbDerivedKey,
    ULONG cbDerivedKey,
    ULONG* pcbResult,
    ULONG dwFlags) {
    // Stub pour Linux
    return 0;
}

inline NTSTATUS BCryptDestroySecret(BCRYPT_SECRET_HANDLE hSecret) {
    // Stub pour Linux
    return 0;
}

inline NTSTATUS BCryptEncrypt(
    BCRYPT_KEY_HANDLE hKey,
    PUCHAR pbInput,
    ULONG cbInput,
    void* pPaddingInfo,
    PUCHAR pbIV,
    ULONG cbIV,
    PUCHAR pbOutput,
    ULONG cbOutput,
    ULONG* pcbResult,
    ULONG dwFlags) {
    // Stub pour Linux
    return 0;
}

inline NTSTATUS BCryptDecrypt(
    BCRYPT_KEY_HANDLE hKey,
    PUCHAR pbInput,
    ULONG cbInput,
    void* pPaddingInfo,
    PUCHAR pbIV,
    ULONG cbIV,
    PUCHAR pbOutput,
    ULONG cbOutput,
    ULONG* pcbResult,
    ULONG dwFlags) {
    // Stub pour Linux
    return 0;
}

// Fonctions stub pour Linux
inline bool CryptImportKey(HCRYPTPROV hProv, const BYTE* pbData, DWORD dwDataLen, HCRYPTKEY hPubKey, DWORD dwFlags, HCRYPTKEY* phKey) {
    // Stub pour Linux
    return false;
}

inline bool CryptExportKey(HCRYPTKEY hKey, HCRYPTKEY hExpKey, DWORD dwBlobType, DWORD dwFlags, BYTE* pbData, DWORD* pdwDataLen) {
    // Stub pour Linux
    return false;
}

inline bool CryptDestroyKey(HCRYPTKEY hKey) {
    // Stub pour Linux
    return false;
}

inline bool CryptSetKeyParam(HCRYPTKEY hKey, DWORD dwParam, const BYTE* pbData, DWORD dwFlags) {
    // Stub pour Linux
    return false;
}

inline bool CryptEncrypt(HCRYPTKEY hKey, HCRYPTHASH hHash, BOOL Final, DWORD dwFlags, BYTE* pbData, DWORD* pdwDataLen, DWORD dwBufLen) {
    // Stub pour Linux
    return false;
}

inline bool CryptDecrypt(HCRYPTKEY hKey, HCRYPTHASH hHash, BOOL Final, DWORD dwFlags, BYTE* pbData, DWORD* pdwDataLen) {
    // Stub pour Linux
    return false;
}

#endif // !MINI_OS_WINDOWS