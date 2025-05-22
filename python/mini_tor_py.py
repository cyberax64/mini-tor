"""
Module Python pour mini-tor
"""

try:
    from mini_tor import MiniTor
except ImportError:
    raise ImportError("Le module mini_tor n'a pas été correctement installé. Veuillez exécuter 'pip install -e .'")

class TorClient:
    """
    Client Tor pour se connecter aux services cachés (.onion)
    """
    
    def __init__(self):
        """
        Initialise le client Tor
        """
        self._mini_tor = MiniTor()
        self._connected = False
    
    def connect(self, onion_address, port=80):
        """
        Se connecte à une adresse .onion
        
        Args:
            onion_address (str): Adresse .onion (sans le http://)
            port (int, optional): Port à utiliser. Par défaut 80.
        
        Returns:
            bool: True si la connexion a réussi, False sinon
        """
        # Nettoyer l'adresse onion si nécessaire
        if onion_address.startswith('http://'):
            onion_address = onion_address[7:]
        
        if onion_address.endswith('/'):
            onion_address = onion_address[:-1]
        
        if '/' in onion_address:
            onion_address = onion_address.split('/')[0]
        
        try:
            self._mini_tor.connect(onion_address, port)
            self._connected = True
            return True
        except RuntimeError as e:
            print(f"Erreur de connexion: {e}")
            self._connected = False
            return False
    
    def send(self, data):
        """
        Envoie des données au service caché
        
        Args:
            data (str or bytes): Données à envoyer
        
        Returns:
            int: Nombre d'octets envoyés
        
        Raises:
            RuntimeError: Si le client n'est pas connecté
        """
        if not self._connected:
            raise RuntimeError("Non connecté à un service caché")
        
        # Convertir en bytes si nécessaire
        if isinstance(data, str):
            data = data.encode('utf-8')
        
        return self._mini_tor.send(data)
    
    def receive(self, max_size=4096):
        """
        Reçoit des données du service caché
        
        Args:
            max_size (int, optional): Taille maximale des données à recevoir. Par défaut 4096.
        
        Returns:
            bytes: Données reçues
        
        Raises:
            RuntimeError: Si le client n'est pas connecté
        """
        if not self._connected:
            raise RuntimeError("Non connecté à un service caché")
        
        return self._mini_tor.receive(max_size)
    
    def close(self):
        """
        Ferme la connexion
        """
        self._mini_tor.close()
        self._connected = False
    
    def get_routers(self):
        """
        Obtient la liste des routeurs Tor disponibles
        
        Returns:
            list: Liste des routeurs Tor
        """
        return self._mini_tor.get_routers()
    
    def http_get(self, onion_address, path="/", port=80, headers=None):
        """
        Effectue une requête HTTP GET vers un service caché
        
        Args:
            onion_address (str): Adresse .onion (sans le http://)
            path (str, optional): Chemin de la requête. Par défaut "/".
            port (int, optional): Port à utiliser. Par défaut 80.
            headers (dict, optional): En-têtes HTTP supplémentaires.
        
        Returns:
            tuple: (status_code, headers, content)
        """
        # Nettoyer l'adresse onion si nécessaire
        if onion_address.startswith('http://'):
            onion_address = onion_address[7:]
        
        if onion_address.endswith('/'):
            onion_address = onion_address[:-1]
        
        # Extraire le chemin si présent dans l'adresse
        if '/' in onion_address:
            parts = onion_address.split('/', 1)
            onion_address = parts[0]
            if path == "/":
                path = "/" + parts[1]
        
        # Se connecter au service caché
        if not self.connect(onion_address, port):
            return None, None, None
        
        # Construire la requête HTTP
        request = f"GET {path} HTTP/1.1\r\n"
        request += f"Host: {onion_address}\r\n"
        request += "User-Agent: mini-tor/0.1\r\n"
        request += "Connection: close\r\n"
        
        # Ajouter les en-têtes supplémentaires
        if headers:
            for key, value in headers.items():
                request += f"{key}: {value}\r\n"
        
        request += "\r\n"
        
        # Envoyer la requête
        self.send(request)
        
        # Recevoir la réponse
        response = b""
        while True:
            chunk = self.receive()
            if not chunk:
                break
            response += chunk
        
        # Fermer la connexion
        self.close()
        
        # Analyser la réponse
        try:
            # Séparer les en-têtes et le contenu
            headers_end = response.find(b"\r\n\r\n")
            if headers_end == -1:
                return None, None, None
            
            headers_data = response[:headers_end].decode('utf-8', errors='ignore')
            content = response[headers_end + 4:]
            
            # Analyser la première ligne pour obtenir le code de statut
            status_line = headers_data.split('\r\n')[0]
            parts = status_line.split(' ', 2)
            if len(parts) < 3:
                return None, None, None
            
            status_code = int(parts[1])
            
            # Analyser les en-têtes
            headers_dict = {}
            for line in headers_data.split('\r\n')[1:]:
                if not line:
                    continue
                key, value = line.split(':', 1)
                headers_dict[key.strip()] = value.strip()
            
            return status_code, headers_dict, content
        except Exception as e:
            print(f"Erreur lors de l'analyse de la réponse: {e}")
            return None, None, None


# Exemple d'utilisation
if __name__ == "__main__":
    client = TorClient()
    
    # Exemple de requête HTTP vers un service caché
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