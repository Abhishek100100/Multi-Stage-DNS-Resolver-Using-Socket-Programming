#include "allheaders.h"

class ProxyServer {
private:
    map<string, string> cache;
    mutex cacheMutex;
    int serverSocket;
    
    void loadCache() {
        ifstream file("proxy_cache.txt");
        string query, response;
        while (file >> query >> response) {
            cache[query] = response;
        }
        file.close();
        cout << "Proxy: Loaded " << cache.size() << " cached entries" << endl;
    }
    
    void saveCache() {
        ofstream file("proxy_cache.txt");
        for (const auto& entry : cache) {
            file << entry.first << " " << entry.second << endl;
        }
        file.close();
    }
    
    string queryDNSServer(const string& request) {
        int dnsSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (dnsSocket < 0) {
            cout << "Proxy: Failed to create DNS socket" << endl;
            return "DNS_SERVER_ERROR";
        }
        
        struct sockaddr_in dnsAddr;
        memset(&dnsAddr, 0, sizeof(dnsAddr));
        dnsAddr.sin_family = AF_INET;
        dnsAddr.sin_port = htons(DNS_SERVER_PORT);
        inet_pton(AF_INET, "127.0.0.1", &dnsAddr.sin_addr);
        
        if (connect(dnsSocket, (struct sockaddr*)&dnsAddr, sizeof(dnsAddr)) < 0) {
            perror("Proxy: DNS server connection failed");
            close(dnsSocket);
            return "DNS_SERVER_ERROR";
        }
        
        if (send(dnsSocket, request.c_str(), request.length(), 0) < 0) {
            perror("Proxy: Send to DNS server failed");
            close(dnsSocket);
            return "DNS_SERVER_ERROR";
        }
        
        char buffer[BUFFER_SIZE] = {0};
        int bytes = recv(dnsSocket, buffer, BUFFER_SIZE - 1, 0);
        close(dnsSocket);
        
        if (bytes <= 0) {
            cout << "Proxy: No response from DNS server" << endl;
            return "DNS_SERVER_ERROR";
        }
        
        buffer[bytes] = '\0';
        string response(buffer);
        cout << "Proxy: Received from DNS server: " << response << endl;
        return response;
    }
    
public:
    void startServer(int port) {
        loadCache();
        
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            perror("Proxy: Socket creation failed");
            exit(1);
        }
        
        // macOS socket options
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
        
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);
        
        if (::bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            perror("Proxy: Bind failed");
            exit(1);
        }
        
        if (listen(serverSocket, 5) < 0) {
            perror("Proxy: Listen failed");
            exit(1);
        }
        
        cout << "Proxy Server started on port " << port << endl;
        
        while (true) {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            
            if (clientSocket < 0) {
                perror("Proxy: Accept failed");
                continue;
            }
            
            thread clientThread(&ProxyServer::handleClient, this, clientSocket);
            clientThread.detach();
        }
    }
    
    void handleClient(int clientSocket) {
        char buffer[BUFFER_SIZE] = {0};
        int bytes = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes <= 0) {
            cout << "Proxy: No data received from client" << endl;
            close(clientSocket);
            return;
        }
        
        buffer[bytes] = '\0';
        string request(buffer);
        cout << "Proxy: Received request: " << request << endl;
        
        int colonPos = request.find(':');
        if (colonPos == string::npos) {
            string error = "INVALID_REQUEST";
            send(clientSocket, error.c_str(), error.length(), 0);
            close(clientSocket);
            return;
        }
        
        string query = request.substr(colonPos + 1);
        string response;
        
        // Check cache first
        {
            lock_guard<mutex> lock(cacheMutex);
            auto it = cache.find(query);
            if (it != cache.end()) {
                response = it->second;
                cout << "Cache HIT for: " << query << endl;
            }
        }
        
        // If not in cache, query DNS server
        if (response.empty()) {
            cout << "Cache MISS for: " << query << endl;
            response = queryDNSServer(request);
            
            if (response != "NOT_FOUND" && response != "DNS_SERVER_ERROR" && !response.empty()) {
                lock_guard<mutex> lock(cacheMutex);
                cache[query] = response;
                saveCache();
                cout << "Cached new entry: " << query << " -> " << response << endl;
            }
        }
        
        send(clientSocket, response.c_str(), response.length(), 0);
        close(clientSocket);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: ./proxyServer <port>" << endl;
        return 1;
    }
    
    ProxyServer server;
    server.startServer(atoi(argv[1]));
    return 0;
}
