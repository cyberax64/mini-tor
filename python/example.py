#!/usr/bin/env python3
"""
Exemple d'utilisation de l'extension Python pour mini-tor
"""

from mini_tor_py import TorClient
import sys
import argparse

def main():
    """
    Fonction principale
    """
    parser = argparse.ArgumentParser(description="Client Tor pour accéder aux services cachés")
    parser.add_argument("onion", help="Adresse .onion à visiter")
    parser.add_argument("--port", type=int, default=80, help="Port à utiliser (défaut: 80)")
    parser.add_argument("--path", default="/", help="Chemin de la requête (défaut: /)")
    parser.add_argument("--output", help="Fichier de sortie pour le contenu")
    parser.add_argument("--list-routers", action="store_true", help="Afficher la liste des routeurs Tor disponibles")
    
    args = parser.parse_args()
    
    # Créer un client Tor
    client = TorClient()
    
    # Afficher la liste des routeurs si demandé
    if args.list_routers:
        print("Liste des routeurs Tor disponibles:")
        routers = client.get_routers()
        for i, router in enumerate(routers):
            print(f"{i+1}. {router['name']} - {router['ip']}:{router['or_port']}")
        return
    
    # Effectuer une requête HTTP GET
    print(f"Connexion à {args.onion} sur le port {args.port}...")
    status, headers, content = client.http_get(args.onion, args.path, args.port)
    
    if status:
        print(f"Status: {status}")
        print("Headers:")
        for key, value in headers.items():
            print(f"  {key}: {value}")
        print(f"Content length: {len(content)} bytes")
        
        # Afficher un aperçu du contenu
        if not args.output:
            print("Content preview:")
            try:
                preview = content[:500].decode('utf-8', errors='ignore')
                print(preview)
                print("...")
            except:
                print("[Contenu binaire]")
        
        # Enregistrer le contenu dans un fichier si demandé
        if args.output:
            with open(args.output, 'wb') as f:
                f.write(content)
            print(f"Contenu enregistré dans {args.output}")
    else:
        print("La requête a échoué")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nInterruption par l'utilisateur")
        sys.exit(0)
    except Exception as e:
        print(f"Erreur: {e}")
        sys.exit(1)