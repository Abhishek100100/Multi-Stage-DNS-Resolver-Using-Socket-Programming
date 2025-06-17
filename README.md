# Multi-Stage DNS Resolver System

A high-performance 2-tier DNS resolution system implemented in C++ using socket programming, featuring intelligent caching and concurrent request handling.

[![Language](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)
[![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B11)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS-lightgrey.svg)]()
[![License](https://img.shields.io/badge/License-MIT-green.svg)](../two-level-cache-controller/LICENSE)

---

## Features

- **2-Tier Architecture**: Client → Proxy → DNS Server communication model
- **Intelligent Caching**: Proxy-layer caching reduces DNS server load
- **Bidirectional Resolution**: Supports both A-record (domain→IP) and PTR-record (IP→domain) queries
- **Concurrent Processing**: Multi-threaded server design supporting 100+ simultaneous clients
- **Thread-Safe Operations**: Proper synchronization primitives for concurrent access
- **Persistent Cache**: Cache survives server restarts for improved performance

---

## System Architecture

```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│             │    │             │    │             │
│ DNS Client  │◄──►│ Proxy Server│◄──►│ DNS Server  │
│             │    │   (Cache)   │    │ (Database)  │
└─────────────┘    └─────────────┘    └─────────────┘
     Port: *           Port: 8081         Port: 8080
```

### Components

- **DNS Client** ([dnsClient.cpp](dnsClient.cpp)): User interface for DNS queries
- **Proxy Server** ([proxyServer.cpp](proxyServer.cpp)): Caching layer with request forwarding
- **DNS Server** ([dnsServer.cpp](dnsServer.cpp)): Core resolution engine with domain database

---

##  Prerequisites

- **C++ Compiler**: GCC 4.8+ or Clang 3.3+ with C++11 support
- **Operating System**: Linux or macOS
- **Network Tools**: netcat (nc) for testing
- **Make**: GNU Make for compilation

### Installation Commands

**Ubuntu/Debian:**
```
sudo apt-get update
sudo apt-get install build-essential netcat-openbsd
```

**macOS:**
```
xcode-select --install
brew install netcat
```

---

## Installation & Setup

1. **Clone the Repository**
```
git clone https://github.com/Abhishek100100/Multi-Stage-DNS-Resolver-Using-Socket-Programming.
cd Multi-Stage-DNS-Resolver-Using-Socket-Programming
```

2. **Compile the Project**
```
make all
```

3. **Verify Compilation**
```
ls -la dns* proxy*
# Should show: dnsClient dnsServer proxyServer
```

---

##  Quick Start

### 1. Start the DNS Server
```
./dnsServer 8080
```

### 2. Start the Proxy Server
```
./proxyServer 8081
```

### 3. Run the Client
```
./dnsClient 127.0.0.1 8081
```

---

##  Usage Examples

### Domain to IP Resolution
```
./dnsClient 127.0.0.1 8081
# Choose option 1, enter: google.com
# Output: 142.250.191.14
```

### IP to Domain Resolution
```
./dnsClient 127.0.0.1 8081
# Choose option 2, enter: 142.250.191.14
# Output: google.com
```

### Command Line Testing
```
echo "1:google.com" | nc 127.0.0.1 8081
echo "1:google.com" | nc 127.0.0.1 8080
```

---

## 🧪 Load Testing

### Run 100 Concurrent Clients Test
```
chmod +x test_100_clients.sh
./test_100_clients.sh
```

Expected output:
```
=== TEST RESULTS ===
Total clients: 100
Successful requests: 100
Success rate: 100%
Average response time: XXms
```

---

## Configuration

### Database Configuration ([database_mappings.txt](database_mappings.txt))
```
google.com 142.250.191.14
facebook.com 157.240.241.35
github.com 140.82.114.3
stackoverflow.com 151.101.1.69
youtube.com 172.217.164.78
amazon.com 176.32.103.205
```

### Port Configuration
- **DNS Server**: Port 8080 (configurable via command line)
- **Proxy Server**: Port 8081 (configurable via command line)
- **Client**: Connects to proxy server port

### Cache Configuration
- **Cache File**: `proxy_cache.txt`
- **Cache Policy**: Persistent storage
- **Cache Location**: Same directory as executable

---

## Troubleshooting

**"Address already in use" Error**
```
pkill -f dnsServer
pkill -f proxyServer
```

**"Connection refused" Error**
```
lsof -i :8080  # DNS Server
lsof -i :8081  # Proxy Server
```

**"NOT_FOUND" Responses**
```
cat database_mappings.txt
# Restart DNS server to reload database
```

---

## 📁 Project Structure

```
Multi-Stage-DNS-Resolver-Using-Socket-Programming/
├── README.md
├── Makefile
├── allheaders.h
├── dnsClient.cpp
├── dnsServer.cpp
├── proxyServer.cpp
├── database_mappings.txt
├── proxy_cache.txt
├── test_100_clients.sh
```

---
## 📄 License

This project is licensed under the MIT License - see the [LICENSE](../two-level-cache-controller/LICENSE) file for details.

---
## 📚 Technical References

- [RFC 1035](https://tools.ietf.org/html/rfc1035) - Domain Names Implementation
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action)
