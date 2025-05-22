[![Build status](https://ci.appveyor.com/api/projects/status/hjxm9hfjwljab2am?svg=true)](https://ci.appveyor.com/project/wbenny/mini-tor)

# mini-tor

mini-tor est une implémentation minimaliste du protocole Tor en C++, mise à jour pour fonctionner avec le réseau Tor actuel (2025). Cette bibliothèque permet d'accéder au contenu internet et aux services cachés **(.onion)** via le réseau Tor. L'objectif est d'obtenir une taille aussi petite que possible (environ 47 Ko, compressé avec kkrunchy environ 20 Ko), ce qui est réalisé en utilisant Microsoft CryptoAPI/CNG au lieu d'intégrer OpenSSL.

## Fonctionnalités

- Client Tor léger capable de se connecter au réseau Tor actuel
- Support des services cachés (onion services) v2 et v3
- Implémentation du protocole NTOR pour l'établissement de circuits

### Usage

Accéder au contenu internet via mini-tor :
```c
> mini-tor.exe "https://check.torproject.org/"
```

Accéder au contenu d'un service caché via mini-tor :
```c
> mini-tor.exe "http://vww6ybal4bd7szmgncyruucpgfkqahzddi37ktceo3ah7ngmcopnpyyd.onion/"
```

Il peut également accéder au contenu HTTPS :
```c
> mini-tor.exe "https://check.torproject.org/"
> mini-tor.exe "https://www.facebookwkhpilnemxj7asaniu7vnjjbiltxjqhye3mhbshg7kx5tfyd.onion/"
```

Ajoutez -v, -vv ou -vvv pour plus de verbosité :
```c
> mini-tor.exe -v "http://vww6ybal4bd7szmgncyruucpgfkqahzddi37ktceo3ah7ngmcopnpyyd.onion/"
> mini-tor.exe -vv "https://check.torproject.org/"
> mini-tor.exe -vvv "https://www.facebookwkhpilnemxj7asaniu7vnjjbiltxjqhye3mhbshg7kx5tfyd.onion/"
```

Vous pouvez désactiver la journalisation en commentant `#define MINI_LOG_ENABLED` dans `mini/logger.h`.
Cela réduira également considérablement la taille de l'exécutable.

### Description technique

* **mini-tor** prend en charge les protocoles de poignée de main TAP et NTOR.
  * TAP utilise DH standard avec un exposant de 1024 bits.
  * NTOR est un type de poignée de main plus récent qui utilise curve25519.
  * Vous pouvez contrôler quel type de poignée de main est utilisé par défaut en modifiant `preferred_handshake_type` dans `mini/tor/common.h`
* **mini-tor** peut utiliser soit CryptoAPI soit l'API CNG plus récente.
  * Configurable via `mini/crypto/common.h`.
  * Notez que curve25519 est pris en charge par CNG uniquement sur win10+.
    * Une implémentation *curve25519-donna* est incluse. Vous pouvez l'activer en définissant `MINI_CRYPTO_CURVE25519_NAMESPACE` sur `ext` pour l'exécuter sur des systèmes plus anciens.
  * Notez que `cng::dh<>` ne fonctionnera que sur win8.1+, en raison de l'utilisation de `BCRYPT_KDF_RAW_SECRET` pour récupérer le secret partagé.
    * Vous pouvez utiliser `capi::dh<>` en définissant `MINI_CRYPTO_DH_NAMESPACE` sur `capi`.
  * Tout le reste devrait fonctionner correctement sur win7+ (les versions antérieures ne sont pas encore prises en charge).
* **mini-tor** crée des connexions TLS via SCHANNEL (voir `mini/net/ssl_socket.cpp` et `mini/net/detail/ssl_context.cpp`).
* **mini-tor** n'utilise pas la CRT ou STL par défaut, tout est implémenté à partir de zéro.
* Une version plus ancienne de **mini-tor** basée uniquement sur CryptoAPI peut être trouvée dans le tag git **ms_crypto_api** (non maintenue).
* La version mise à jour (2025) ajoute le support pour Linux via OpenSSL et prend en charge les adresses onion v3.

### Création d'un service caché

Voir le fichier `serviceServer.cpp` pour un exemple complet de création d'un serveur HTTP accessible comme service caché Tor.

### Remarques

* Comme indiqué ci-dessus, cette application est une preuve de concept.
* Vous pouvez parfois vous attendre à un comportement bogué.
* Ce n'est définitivement pas un remplacement sécurisé complet pour Tor.

### Compilation

#### Windows
Compilez **mini-tor** à l'aide de Visual Studio 2017 ou supérieur. Le fichier solution est inclus. Aucune autre dépendance n'est requise.

#### Linux
```bash
mkdir build
cd build
cmake ..
make
```

### Licence

Tout le code de ce dépôt est open-source sous la licence MIT. Voir le fichier **LICENSE.txt** dans ce dépôt.


Si vous trouvez ce projet intéressant, vous pouvez m'offrir un café

```
  BTC 3GwZMNGvLCZMi7mjL8K6iyj6qGbhkVMNMF
  LTC MQn5YC7bZd4KSsaj8snSg4TetmdKDkeCYk
```
