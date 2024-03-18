#ifndef PROXY_H
#define PROXY_H

#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>
#include <fstream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "request.h"
#include "response.h"
#include <sstream>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <thread>
#include <utility>
#include "logger.h"
#include "cache.h"
#include <time.h>
#include <utility>

class proxy{
    const char * port;
    int mySocket_fd;
    logger log;
    cache Cache;
public:
    proxy(const char * _port);
    void start();
    void initializeProxy();
    void handleRequest(int client_fd, std::string clientIP);
    int buildConnection(std::string hostName, std::string port);
    std::pair<int,std::string> acceptConnection(int socket_fd);
    int initializeListenSocket();
    int initializeConnectSocket(std::string hostName, std::string port);
    response recvResponseFromSocket(int server_fd);
    response openChannelForHTTPConnect(request request,int client_fd);
    response forwardToServer(request request);
    void forwardToClient(response response,int client_fd);

    bool supportCache(request request);
    bool cache_revalidation(request request);
    bool cache_valid(request request);
};

#endif