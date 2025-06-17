#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <cstring>

using namespace std;

#define BUFFER_SIZE 1024

class DNSServer {
private:
    map<string, string> domainToIP;
    map<string, string> ipToDomain;
    int serverSocket;
    
    void loadDatabase() {
        cout << "DNS Server: Attempting to load database_mappings.txt..." << endl;
        ifstream file("database_mappings.txt");
        
        if (!file.is_open()) {
            cout << "DNS Server: ERROR - Cannot open database_mappings.txt" << endl;
            return;
        }
        
        string domain, ip;
        int count = 0;
        while (file >> domain >> ip) {
            domainToIP[domain] = ip;
            ipToDomain[ip] = domain;
            count++;
            cout << "DNS Server: Loaded mapping " << count << ": " << domain << " -> " << ip << endl;
        }
        file.close();
        
        cout << "DNS Server: Successfully loaded " << domainToIP.size() << " domain mappings" << endl;
        
        // Print all loaded domains for verification
        cout << "DNS Server: Available domains: ";
        for (const auto& entry : domainToIP) {
            cout << entry.first << " ";
        }
        cout << endl;
    }
    
public:
    void startServer(int port) {
        loadDatabase();
        
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            perror("DNS Server: Socket creation failed");
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
            perror("DNS Server: Bind failed");
            exit(1);
        }
        
        if (listen(serverSocket, 5) < 0) {
            perror("DNS Server: Listen failed");
            exit(1);
        }
        
        cout << "DNS Server: Started successfully on port " << port << endl;
        
        while (true) {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            
            if (clientSocket < 0) {
                perror("DNS Server: Accept failed");
                continue;
            }
            
            cout << "DNS Server: New client connected" << endl;
            thread clientThread(&DNSServer::handleClient, this, clientSocket);
            clientThread.detach();
        }
    }
    
    void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE] = {0};
    int bytes = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes <= 0) {
        cout << "DNS Server: No data received from client" << endl;
        close(clientSocket);
        return;
    }
    
    buffer[bytes] = '\0';
    string request(buffer);
    
    // REMOVE TRAILING WHITESPACE (newlines, spaces, etc.)
    request.erase(request.find_last_not_of(" \n\r\t") + 1);
    
    cout << "DNS Server: Received request: '" << request << "'" << endl;
    
    int colonPos = request.find(':');
    if (colonPos == string::npos) {
        cout << "DNS Server: Invalid request format (no colon)" << endl;
        string error = "INVALID_REQUEST";
        send(clientSocket, error.c_str(), error.length(), 0);
        close(clientSocket);
        return;
    }
    
    int requestType = stoi(request.substr(0, colonPos));
    string query = request.substr(colonPos + 1);
    
    // ALSO CLEAN THE QUERY STRING
    query.erase(query.find_last_not_of(" \n\r\t") + 1);
    
    cout << "DNS Server: Request type: " << requestType << ", Query: '" << query << "'" << endl;
    
    string response;
    if (requestType == 1) { // Domain to IP
        auto it = domainToIP.find(query);
        if (it != domainToIP.end()) {
            response = it->second;
            cout << "DNS Server: Found mapping: " << query << " -> " << response << endl;
        } else {
            response = "NOT_FOUND";
            cout << "DNS Server: No mapping found for domain: '" << query << "'" << endl;
        }
    } else if (requestType == 2) { // IP to Domain
        auto it = ipToDomain.find(query);
        response = (it != ipToDomain.end()) ? it->second : "NOT_FOUND";
    } else {
        response = "INVALID_REQUEST_TYPE";
    }
    
    cout << "DNS Server: Sending response: '" << response << "'" << endl;
    send(clientSocket, response.c_str(), response.length(), 0);
    close(clientSocket);
}

};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: ./dnsServer <port>" << endl;
        return 1;
    }
    
    DNSServer server;
    server.startServer(atoi(argv[1]));
    return 0;
}
