#include <mini/logger.h>
#include <mini/console.h>
#include <mini/crypto/random.h>
#include <mini/io/file.h>
#include <mini/io/file_stream.h>
#include <mini/net/tcp_listener.h>
#include <mini/net/tcp_socket.h>
#include <mini/tor/tor_client.h>
#include <mini/tor/tor_stream.h>
#include <mini/tor/hidden_service.h>

#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <functional>
#include <atomic>

using namespace mini;

// Structure pour stocker les informations de la requête HTTP
struct HttpRequest {
    string method;
    string path;
    string version;
    collections::dictionary<string, string> headers;
    string body;
};

// Structure pour stocker les informations de la réponse HTTP
struct HttpResponse {
    string version = "HTTP/1.1";
    int status_code = 200;
    string status_message = "OK";
    collections::dictionary<string, string> headers;
    string body;

    string to_string() const {
        string result = version + " " + std::to_string(status_code) + " " + status_message + "\r\n";
        
        // Ajouter les en-têtes
        for (auto& header : headers) {
            result += header.first + ": " + header.second + "\r\n";
        }
        
        // Ajouter la longueur du contenu si elle n'est pas déjà définie
        if (!headers.contains("Content-Length")) {
            result += "Content-Length: " + std::to_string(body.get_size()) + "\r\n";
        }
        
        // Ligne vide pour séparer les en-têtes du corps
        result += "\r\n";
        
        // Ajouter le corps
        result += body;
        
        return result;
    }
};

// Fonction pour analyser une requête HTTP
HttpRequest parse_http_request(const string& request_str) {
    HttpRequest request;
    
    // Diviser la requête en lignes
    auto lines = request_str.split("\r\n");
    
    if (lines.get_size() > 0) {
        // Analyser la ligne de requête (méthode, chemin, version)
        auto request_line = lines[0].split(" ");
        if (request_line.get_size() >= 3) {
            request.method = request_line[0];
            request.path = request_line[1];
            request.version = request_line[2];
        }
        
        // Analyser les en-têtes
        size_t i = 1;
        for (; i < lines.get_size(); i++) {
            if (lines[i].is_empty()) {
                i++; // Passer la ligne vide qui sépare les en-têtes du corps
                break;
            }
            
            auto header_parts = lines[i].split(": ");
            if (header_parts.get_size() >= 2) {
                request.headers.add(header_parts[0], header_parts[1]);
            }
        }
        
        // Récupérer le corps de la requête
        for (; i < lines.get_size(); i++) {
            request.body += lines[i];
            if (i < lines.get_size() - 1) {
                request.body += "\r\n";
            }
        }
    }
    
    return request;
}

// Classe pour gérer un serveur HTTP simple
class SimpleHttpServer {
private:
    net::tcp_listener _listener;
    std::atomic<bool> _running;
    std::function<HttpResponse(const HttpRequest&)> _request_handler;
    std::vector<std::thread> _worker_threads;
    
public:
    SimpleHttpServer(uint16_t port, std::function<HttpResponse(const HttpRequest&)> handler)
        : _listener(port)
        , _running(false)
        , _request_handler(handler) {
    }
    
    ~SimpleHttpServer() {
        stop();
    }
    
    void start() {
        if (_running) {
            return;
        }
        
        _running = true;
        _listener.start();
        
        // Créer un thread pour accepter les connexions
        _worker_threads.emplace_back([this]() {
            while (_running) {
                try {
                    auto client = _listener.accept();
                    
                    // Créer un thread pour gérer cette connexion
                    _worker_threads.emplace_back([this, client = std::move(client)]() mutable {
                        handle_client(std::move(client));
                    });
                } catch (const std::exception& e) {
                    if (_running) {
                        mini_warning("Erreur lors de l'acceptation d'une connexion: %s", e.what());
                    }
                }
            }
        });
        
        mini_info("Serveur HTTP démarré sur le port %u", _listener.get_port());
    }
    
    void stop() {
        if (!_running) {
            return;
        }
        
        _running = false;
        _listener.stop();
        
        // Attendre que tous les threads se terminent
        for (auto& thread : _worker_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        _worker_threads.clear();
        
        mini_info("Serveur HTTP arrêté");
    }
    
private:
    void handle_client(net::tcp_socket client) {
        try {
            // Tampon pour recevoir les données
            byte_buffer buffer(4096);
            
            // Lire les données du client
            size_t bytes_read = client.receive(buffer);
            
            if (bytes_read > 0) {
                // Convertir les données en chaîne
                string request_str(reinterpret_cast<char*>(buffer.get_buffer()), bytes_read);
                
                // Analyser la requête HTTP
                HttpRequest request = parse_http_request(request_str);
                
                // Traiter la requête et obtenir une réponse
                HttpResponse response = _request_handler(request);
                
                // Envoyer la réponse au client
                string response_str = response.to_string();
                client.send(byte_buffer_ref(response_str.get_buffer(), response_str.get_size()));
            }
        } catch (const std::exception& e) {
            mini_warning("Erreur lors du traitement d'une connexion client: %s", e.what());
        }
        
        // Fermer la connexion
        client.close();
    }
};

// Fonction principale pour créer un service caché Tor avec un serveur HTTP
int main(int argc, char* argv[]) {
    console::set_title("Service caché Tor avec serveur HTTP");
    console::enable_colors();
    
    mini_info("Initialisation du service caché Tor...");
    
    // Port local sur lequel le serveur HTTP écoutera
    uint16_t local_port = 8080;
    
    // Port virtuel du service caché Tor
    uint16_t virtual_port = 80;
    
    // Créer un gestionnaire de requêtes HTTP simple
    auto http_handler = [](const HttpRequest& request) -> HttpResponse {
        HttpResponse response;
        
        mini_info("Requête reçue: %s %s", request.method.c_str(), request.path.c_str());
        
        // Page d'accueil
        if (request.path == "/" || request.path == "/index.html") {
            response.headers.add("Content-Type", "text/html; charset=utf-8");
            response.body = "<!DOCTYPE html>\n"
                           "<html>\n"
                           "<head>\n"
                           "    <title>Service caché Tor</title>\n"
                           "</head>\n"
                           "<body>\n"
                           "    <h1>Bienvenue sur mon service caché Tor!</h1>\n"
                           "    <p>Ce serveur est accessible uniquement via le réseau Tor.</p>\n"
                           "</body>\n"
                           "</html>";
        }
        // Page À propos
        else if (request.path == "/about") {
            response.headers.add("Content-Type", "text/html; charset=utf-8");
            response.body = "<!DOCTYPE html>\n"
                           "<html>\n"
                           "<head>\n"
                           "    <title>À propos - Service caché Tor</title>\n"
                           "</head>\n"
                           "<body>\n"
                           "    <h1>À propos</h1>\n"
                           "    <p>Ce service caché Tor a été créé avec mini-tor.</p>\n"
                           "    <p><a href=\"/\">Retour à l'accueil</a></p>\n"
                           "</body>\n"
                           "</html>";
        }
        // Page non trouvée
        else {
            response.status_code = 404;
            response.status_message = "Not Found";
            response.headers.add("Content-Type", "text/html; charset=utf-8");
            response.body = "<!DOCTYPE html>\n"
                           "<html>\n"
                           "<head>\n"
                           "    <title>404 - Page non trouvée</title>\n"
                           "</head>\n"
                           "<body>\n"
                           "    <h1>404 - Page non trouvée</h1>\n"
                           "    <p>La page demandée n'existe pas.</p>\n"
                           "    <p><a href=\"/\">Retour à l'accueil</a></p>\n"
                           "</body>\n"
                           "</html>";
        }
        
        return response;
    };
    
    try {
        // Démarrer le serveur HTTP local
        SimpleHttpServer http_server(local_port, http_handler);
        http_server.start();
        
        // Créer un client Tor
        tor::tor_client tor;
        
        // Attendre que le client Tor soit prêt
        mini_info("Connexion au réseau Tor...");
        tor.wait_until_ready();
        mini_info("Connecté au réseau Tor!");
        
        // Créer un service caché Tor
        mini_info("Création du service caché...");
        
        // Générer une clé privée pour le service caché ou en charger une existante
        byte_buffer private_key;
        string key_file = "hidden_service_key.bin";
        
        if (io::file::exists(key_file)) {
            // Charger la clé existante
            io::file_stream fs(key_file, io::file_access::read);
            private_key.resize(fs.get_size());
            fs.read(private_key);
            mini_info("Clé privée chargée depuis %s", key_file.c_str());
        } else {
            // Générer une nouvelle clé
            private_key = crypto::random::generate_random_bytes(32);
            
            // Sauvegarder la clé pour une utilisation future
            io::file_stream fs(key_file, io::file_access::write);
            fs.write(private_key);
            mini_info("Nouvelle clé privée générée et sauvegardée dans %s", key_file.c_str());
        }
        
        // Créer le service caché avec la clé privée
        tor::hidden_service service(tor, private_key);
        
        // Ajouter un port virtuel qui redirige vers le serveur HTTP local
        service.add_port(virtual_port, "127.0.0.1", local_port);
        
        // Obtenir l'adresse onion du service caché
        string onion_address = service.get_hostname();
        
        mini_success("Service caché créé avec succès!");
        mini_success("Adresse onion: %s.onion", onion_address.c_str());
        mini_success("Le service est accessible à l'adresse: http://%s.onion", onion_address.c_str());
        
        // Garder le programme en cours d'exécution
        mini_info("Appuyez sur Entrée pour quitter...");
        std::string input;
        std::getline(std::cin, input);
        
        // Arrêter le serveur HTTP
        http_server.stop();
        
    } catch (const std::exception& e) {
        mini_error("Erreur: %s", e.what());
        return 1;
    }
    
    return 0;
}