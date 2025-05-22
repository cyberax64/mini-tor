# Extension Python pour mini-tor

Cette extension permet d'utiliser la bibliothèque mini-tor depuis Python pour accéder aux services cachés Tor (.onion).

## Installation

Pour installer l'extension, exécutez la commande suivante depuis le répertoire `python` :

```bash
pip install -e .
```

## Prérequis

### Sur Linux
- Un compilateur C++ compatible avec C++17
- Python 3.6 ou supérieur avec les headers de développement
- OpenSSL (libssl-dev)
- pthread

### Sur Windows
- Visual Studio 2019 ou supérieur avec support C++17
- Python 3.6 ou supérieur avec les headers de développement

## Utilisation

Voici un exemple simple pour accéder à un service caché Tor :

```python
from mini_tor_py import TorClient

# Créer un client Tor
client = TorClient()

# Effectuer une requête HTTP GET vers un service caché
status, headers, content = client.http_get("p53lf57qovyuvwsc6xnrppyply3vtqm7l6pcobkmyqsiofyeznfu5uqd.onion")

if status:
    print(f"Status: {status}")
    print("Headers:")
    for key, value in headers.items():
        print(f"  {key}: {value}")
    print(f"Content length: {len(content)} bytes")
    print("Content preview:")
    print(content[:200])
else:
    print("La requête a échoué")
```

Pour une utilisation plus avancée, vous pouvez utiliser les méthodes de bas niveau :

```python
from mini_tor_py import TorClient

# Créer un client Tor
client = TorClient()

# Se connecter à un service caché
client.connect("p53lf57qovyuvwsc6xnrppyply3vtqm7l6pcobkmyqsiofyeznfu5uqd.onion", 80)

# Envoyer une requête HTTP
request = "GET / HTTP/1.1\r\nHost: p53lf57qovyuvwsc6xnrppyply3vtqm7l6pcobkmyqsiofyeznfu5uqd.onion\r\nConnection: close\r\n\r\n"
client.send(request)

# Recevoir la réponse
response = b""
while True:
    chunk = client.receive()
    if not chunk:
        break
    response += chunk

# Afficher la réponse
print(response.decode('utf-8', errors='ignore'))

# Fermer la connexion
client.close()
```

## API

### Classe `TorClient`

#### `__init__()`
Initialise le client Tor.

#### `connect(onion_address, port=80)`
Se connecte à une adresse .onion.

- `onion_address` (str): Adresse .onion (sans le http://)
- `port` (int, optional): Port à utiliser. Par défaut 80.
- Retourne: `True` si la connexion a réussi, `False` sinon

#### `send(data)`
Envoie des données au service caché.

- `data` (str or bytes): Données à envoyer
- Retourne: Nombre d'octets envoyés

#### `receive(max_size=4096)`
Reçoit des données du service caché.

- `max_size` (int, optional): Taille maximale des données à recevoir. Par défaut 4096.
- Retourne: Données reçues (bytes)

#### `close()`
Ferme la connexion.

#### `get_routers()`
Obtient la liste des routeurs Tor disponibles.

- Retourne: Liste des routeurs Tor

#### `http_get(onion_address, path="/", port=80, headers=None)`
Effectue une requête HTTP GET vers un service caché.

- `onion_address` (str): Adresse .onion (sans le http://)
- `path` (str, optional): Chemin de la requête. Par défaut "/".
- `port` (int, optional): Port à utiliser. Par défaut 80.
- `headers` (dict, optional): En-têtes HTTP supplémentaires.
- Retourne: Tuple (status_code, headers, content)