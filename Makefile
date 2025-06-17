CC = g++
CFLAGS = -std=c++11 -pthread

all: dnsServer proxyServer dnsClient

dnsServer: dnsServer.cpp
	$(CC) $(CFLAGS) -o dnsServer dnsServer.cpp

proxyServer: proxyServer.cpp
	$(CC) $(CFLAGS) -o proxyServer proxyServer.cpp

dnsClient: dnsClient.cpp
	$(CC) $(CFLAGS) -o dnsClient dnsClient.cpp

clean:
	rm -f dnsServer proxyServer dnsClient

.PHONY: all clean
