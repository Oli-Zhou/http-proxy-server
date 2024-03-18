#include "proxy.h"

const std::string LOG_FILE_PATH = "/var/log/erss/proxy.log";

proxy::proxy(const char * _port) : port(_port), log(LOG_FILE_PATH),Cache(cache(10)) {}

void proxy::initializeProxy(){
    this->mySocket_fd = initializeListenSocket();
}
void proxy::start(){
    initializeProxy();
    //accept new connections & create thread to handle each connection
    while(1){
        std::pair<int,std::string> acceptResult = acceptConnection(this->mySocket_fd);
        int client_fd = acceptResult.first;
        std::string clientIP = acceptResult.second;
        std::thread t;
        try{
            t = std::thread(&proxy::handleRequest, this, client_fd, clientIP);
        }
        catch(std::runtime_error& e){
            t.detach();
            continue;
        }
        t.detach();
    } 
    close(this->mySocket_fd);
}
response proxy::forwardToServer(request request){
    std::string hostName = request.getHost();
    std::string port = request.getPort();
    std::string plainText = request.to_string();
    // request.print();
    int server_fd = initializeConnectSocket(hostName,port);
    send(server_fd,plainText.c_str(),plainText.size(), 0);
    response response = recvResponseFromSocket(server_fd);
    // response.print();
    // std::cout<<len<<std::endl;
    close(server_fd);
    // response.printHeader();
    return response;
}
/**
 * @brief handle HTTP GET,POST,CONNECT request after successfully receiving socket connection
*/


void proxy::forwardToClient(response response,int client_fd){
    std::string responsePlainText = response.to_string();
    send(client_fd,responsePlainText.c_str(),responsePlainText.size(), 0);
}

void proxy::handleRequest(int client_fd, std::string clientIP){
    char buffer[65536] = { 0 };
    int len = recv(client_fd, buffer, sizeof(buffer),0);
    if(len<=0){
        return ;
    }
    request request(buffer,len);
    request.setClientIP(clientIP);
    log.logRequest(request);
    if(request.getMethod() == "CONNECT"){
        log.logRequestContactWithOriginServer(request);
        response connectResponse = openChannelForHTTPConnect(request,client_fd);
        log.logResponseContactWithOriginServer(request,connectResponse);
        //TODO: log
    }
    else if(request.getMethod() == "POST"){
        log.logRequestContactWithOriginServer(request);
        response response = forwardToServer(request);
        forwardToClient(response,client_fd);
        log.logResponseContactWithOriginServer(request,response);
        log.logRespondingToClient(request,response);
    }
    else if(request.getMethod() == "GET"){
        //not in cache 
        if(!Cache.hit(request)){
            log.logNotInCache(request);
            // std::cout << "not in cache" << std::endl;
            log.logRequestContactWithOriginServer(request);
            response response = forwardToServer(request);
            log.logResponseContactWithOriginServer(request,response);
            if(response.getStatus() == "200"){
                if(response.supportCache()){
                    Cache.insert(request,response);
                    if(response.revalidation_needed){
                        log.logCachedButNeedRevalidation(request);
                    }
                    else if(response.max_age){
                        log.logCachedButExpired(request, response);
                    }
                    
                }
                else{
                    if(response.no_store){
                        log.logNotCacheable(request, "no-store cache control");
                    }
                    else{
                        log.logNotCacheable(request, "private cache control");
                    }
                }
            }
            forwardToClient(response,client_fd);
            log.logRespondingToClient(request,response);
        }
        // in cache
        else {

            response cachedResponse = Cache.lookup(request);
            std::string cacheControl = cachedResponse.get("Cache-Control");
            bool mustRevalidate = cacheControl.find("must-revalidate")!=std::string::npos;
            
            //check revalidation
            if(Cache.needRevalidation(request)){
                // std::cout << "in cache, requires validation" << std::endl;
                log.loginCacheButNeedValidation(request);
            }
            //check expiration
            else if(Cache.checkExpiration(request)){
                // std::cout << "in cache, but expired at " << cachedResponse.get("Expires") << std::endl;
                log.loginCacheButExpired(request, cachedResponse);
            }
            //valid, send cached response back to client
            else{
                log.loginCacheAndValid(request);
                response cachedResponse = Cache.lookup(request);
                log.logRespondingToClient(request,cachedResponse);
                forwardToClient(cachedResponse,client_fd);
                close(client_fd);
                return;
            }
            //revalidation process
            if(cachedResponse.get("ETag")!=""
                ||cachedResponse.get("Last_Modified")!=""
                ||mustRevalidate
                ||cachedResponse.get("Expires")!=""){
                if(cachedResponse.get("ETag")!=""){
                    request.put("If-None-Match",cachedResponse.get("ETag"));
                }
                else if (cachedResponse.get("Last_Modified")!=""){
                    request.put("If-Modified-Since",cachedResponse.get("Last_Modified"));
                }


                log.logRequestContactWithOriginServer(request);
                response ack = forwardToServer(request);
                log.logResponseContactWithOriginServer(request,ack);
                if(ack.getStatus()=="304"){
                    forwardToClient(cachedResponse,client_fd); 
                    log.logRespondingToClient(request,cachedResponse);
                }
                else if(ack.getStatus()=="200"){
                    if(ack.supportCache()){
                        Cache.insert(request,ack);
                        if(ack.revalidation_needed){
                            log.logCachedButNeedRevalidation(request);
                        }
                        else if(ack.max_age){
                            log.logCachedButExpired(request, ack);
                        }
                        
                    }
                    else{
                        if(ack.no_store){
                            log.logNotCacheable(request, "no-store cache control");
                        }
                        else{
                            log.logNotCacheable(request, "private cache control");
                        }
                    }
                    forwardToClient(ack,client_fd);
                    log.logRespondingToClient(request,ack);
                }
                else{
                    forwardToClient(ack,client_fd);
                    log.logRespondingToClient(request,cachedResponse);   
                }


                if(cachedResponse.get("ETag")!=""){
                    request.erase("If-None-Match");
                }
                else if (cachedResponse.get("Last_Modified")!=""){
                    request.erase("If-Modified-Since");
                }
            }

            // log.logRequestContactWithOriginServer(request);
            // response ack = forwardToServer(request);
            // log.logResponseContactWithOriginServer(request,ack);

            // if(ack.getStatus()=="304"){
            //     forwardToClient(cachedResponse,client_fd); 
            //     log.logRespondingToClient(request,cachedResponse);   
            // }
            // else if(ack.getStatus()=="200"){
            //     log.logRespondingToClient(request,ack);
            //     forwardToClient(ack,client_fd);
            //     std::string msg = "";
            //     if(ack.supportCache()){
            //         Cache.insert(request,ack);
            //     }
            //     else{
            //         log.logNotCacheable(request,msg);
            //     }
            // }
            // forwardToClient(cachedResponse, client_fd);
            // log.logRespondingToClient(request, cachedResponse);

        }
    }
    close(client_fd);
}

/**
 * @brief open I/O multiplexing channel for client_fd and server_fd for HTTP CONNECT request.
*/
response proxy::openChannelForHTTPConnect(request request,int client_fd){
    int server_fd = initializeConnectSocket(request.getHost(),request.getPort());
    std::string MsgOK = "HTTP/1.1 200 Connection Established\r\nConnection: close\r\n\r\n";
    int fdMax = server_fd;
    fd_set fdSet;
    fd_set read_fds;
    FD_ZERO(&fdSet);
    FD_SET(server_fd, &fdSet);
    FD_SET(client_fd, &fdSet);
    response returnResponse = response(MsgOK);
    log.logRespondingToClient(request,returnResponse);
    send(client_fd,MsgOK.c_str(),MsgOK.size(), 0);
    std::vector<char> buffer(65536);
    while(1){
        read_fds = fdSet;
        if ((select(fdMax + 1, &read_fds, NULL, NULL, NULL) == -1)) {
            throw std::runtime_error("Failed select");
        }
        // client -> proxy -> server
        if (FD_ISSET(client_fd, &read_fds)) {
            buffer.clear();
            // std::cout << "-----------client----------" << std::endl;
            int len = recv(client_fd, buffer.data(), 65536, 0);
            // std::cout<<buffer.data()<<std::endl;
            // std::cout << "---------------------------" << std::endl;
            if (len <= 0) {
                break;
            }
            send(server_fd, buffer.data(), len, 0);
        }
        // server -> proxy -> client
        if (FD_ISSET(server_fd, &read_fds)) {
            buffer.clear();
            // std::cout << "-----------server----------" << std::endl;
            int len = recv(server_fd, buffer.data(), 65536, 0);
            // std::cout<<buffer.data()<<std::endl;
            // std::cout << "---------------------------" << std::endl;
            if (len <= 0) {
                break;
            }
            send(client_fd, buffer.data(), len, 0);
        }
    }
    log.logTunnelClosed(request);
    close(server_fd);
    return returnResponse;
}
response proxy::recvResponseFromSocket(int server_fd){
    std::vector<char> buffer(65536) ; 
    std::string receivedData;
    std::string header = "";
    std::string body = "";
    // string receivedData;
    // std::cout<<"receive start!"<< std::endl;
    int contentLen = 0;
    while(1){
        buffer.clear();
        int len = recv(server_fd,buffer.data(),65536,0);
        if(len<=0){
            break;
        }
        receivedData+=std::string(buffer.begin(),buffer.begin()+len);
        int pos = receivedData.find("\r\n\r\n");
        if(pos!=std::string::npos){
            header = receivedData.substr(0,pos+4);
            body = receivedData.substr(pos+4);
            // cout<<respond;
            int pos2 = header.find("Content-Length: ");
            if(pos2!=std::string::npos){
                contentLen = stoi(header.substr(pos2+16));
            }
            break;
        }
    }
    int rmnLen = contentLen - body.size();
    while(rmnLen != 0){
        buffer.clear();
        if(rmnLen>0){
            int len = recv(server_fd,buffer.data(),65536,0);
            // std::cout<<"len="<< len<<std::endl;
            if(len<=0){
                // std::cout <<"recv fail!"<< std::endl;
                break;
            }
            body += std::string(buffer.begin(),buffer.begin()+len);
            rmnLen -=len;
        }
    }
    // std::cout<<"body end!"<< std::endl;
    // std::cout<<"receive finished!"<< std::endl;
    close(server_fd);
    return response(header,body);
}
int proxy::buildConnection(std::string hostName,std::string port){
    return 0;
}
std::pair<int,std::string> proxy::acceptConnection(int socket_fd){
    struct sockaddr_in addr;
    socklen_t addrsize = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(std::stoi(port));
    int client_fd = accept(socket_fd, (struct sockaddr*)&addr,(socklen_t*)&addrsize);
    if (client_fd < 0) {
        throw std::runtime_error("Failed accept");  
    }
    // std::cout<<"connect succeed"<<std::endl;
    // char ip_addr[INET_ADDRSTRLEN];
    // inet_ntop(AF_INET, &(addr.sin_addr), ip_addr, INET_ADDRSTRLEN);
    // std::string ip_addr_str(ip_addr);
    getpeername(client_fd, (struct sockaddr*)&addr, &addrsize);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
    std::string ip_addr_str(ip);

    return {client_fd, ip_addr_str};
}

/**
 * @brief build connection with server, and return the socket_fd
 * @return socket_fd
 */
int proxy::initializeConnectSocket(std::string hostName, std::string port){
    struct addrinfo addr;
    struct addrinfo * addrList,*it;

    memset(&addr, 0, sizeof(addr));


    addr.ai_family = AF_UNSPEC;
    addr.ai_socktype = SOCK_STREAM;
    addr.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(hostName.c_str(), port.c_str(), &addr, &addrList);
    // std::cout << "status:"<< status << std::endl;
    if (status != 0) {
        std::cout << "error:" << gai_strerror(status) << std::endl;
        std::cout << "cannot get address info:" << std::endl <<"(hostName:'" << hostName.c_str()<<"'"<< std::endl<<", port:" << port.c_str() << ")" << std::endl;
        return -1;
    }
    int socket_fd;
    for(it = addrList;it!=nullptr;it = it->ai_next){  
        socket_fd = socket(it->ai_family,
                        it->ai_socktype,
                        it->ai_protocol);

        // cout<<"socket_fd:"<<socket_fd<<endl;
        if (socket_fd == -1) {
            continue;
        }
        // int flags = fcntl(socket_fd, F_GETFL, 0);
        // fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK | AI_CANONNAME);
        int yes = 1;
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            throw std::runtime_error("Failed setsocket");  
        }
        // cout<<"ip:"<<addrList->ai_addr<<endl;
        status = connect(socket_fd, it->ai_addr, it->ai_addrlen);
        if (status != -1) {
            break;
        }
        close(socket_fd);
    }  
    if(it == nullptr){
        throw std::runtime_error("Failed connection");  
    }
    return socket_fd;
}


/**
 * @brief build listen socket, and return the socket_fd
 * @return socket_fd
 */
int proxy::initializeListenSocket(){
    struct sockaddr_in addr;
    int opt = 1;
    int addrlen = sizeof(addr);
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        throw std::runtime_error("Failed socket initialization");  
    }
 
    if (setsockopt(socket_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        throw std::runtime_error("Failed setsocket");  
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(std::stoi(port));

    if (bind(socket_fd, (struct sockaddr*)&addr,sizeof(addr)) < 0) {
        throw std::runtime_error("Failed bind");  
    }
    if (listen(socket_fd, 3) < 0) {
        throw std::runtime_error("Failed listen");  
    }
    return socket_fd;
}