#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

#define BUFFER_SIZE 1024

class DNSClient {
private:
    int proxySocket;
    
    bool connectToProxy(const char* ip, int port) {
        proxySocket = socket(AF_INET, SOCK_STREAM, 0);
        if (proxySocket < 0) {
            perror("Socket creation failed");
            return false;
        }
        
        struct sockaddr_in proxyAddr;
        memset(&proxyAddr, 0, sizeof(proxyAddr));
        proxyAddr.sin_family = AF_INET;
        proxyAddr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, ip, &proxyAddr.sin_addr) <= 0) {
            perror("Invalid address");
            close(proxySocket);
            return false;
        }
        
        if (connect(proxySocket, (struct sockaddr*)&proxyAddr, sizeof(proxyAddr)) < 0) {
            perror("Connection failed");
            close(proxySocket);
            return false;
        }
        
        return true;
    }
    
public:
    void startClient(const char* proxyIP, int proxyPort) {
        int choice;
        string query;
        
        while (true) {
            cout << "\n=== DNS Resolver Client ===" << endl;
            cout << "1. Domain to IP" << endl;
            cout << "2. IP to Domain" << endl;
            cout << "3. Exit" << endl;
            cout << "Enter choice: ";
            cin >> choice;
            
            if (choice == 3) break;
            
            cout << "Enter query: ";
            cin >> query;
            
            if (!connectToProxy(proxyIP, proxyPort)) {
                cout << "Failed to connect to proxy server" << endl;
                continue;
            }
            
            string request = to_string(choice) + ":" + query;
            if (send(proxySocket, request.c_str(), request.length(), 0) < 0) {
                perror("Send failed");
                close(proxySocket);
                continue;
            }
            
            char buffer[BUFFER_SIZE] = {0};
            int bytes = recv(proxySocket, buffer, BUFFER_SIZE - 1, 0);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                cout << "Response: " << buffer << endl;
            } else {
                cout << "No response received" << endl;
            }
            
            close(proxySocket);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: ./dnsClient <proxy_ip> <proxy_port>" << endl;
        return 1;
    }
    
    DNSClient client;
    client.startClient(argv[1], atoi(argv[2]));
    return 0;
}
