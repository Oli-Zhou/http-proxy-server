#include "request.h"

request::request(char* httpRequestPlainText,int requestLen) : id(next_id++){
    min_fresh = 0;
    max_stale = 0;
    parseRequest(httpRequestPlainText,requestLen);
}


void request::parseRequest(char* httpRequestPlainText,int requestLen){
    std::string httpRequestPlainTextString(httpRequestPlainText,requestLen);
    int pos = httpRequestPlainTextString.find("\r\n\r\n");
    if(pos != std::string::npos){
        std::string headersStr = httpRequestPlainTextString.substr(0,pos+4);
        body = httpRequestPlainTextString.substr(pos+4);
        std::stringstream ss(headersStr);
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(ss, line)) {
            lines.push_back(line);
            if(line.find("min-fresh")!=std::string::npos){
                int pos1 = line.find("=");
                min_fresh = atoi(line.substr(pos1+1).c_str());
            }
            else if(line.find("max-stale")!=std::string::npos){
                int pos1 = line.find("=");
                max_stale = atoi(line.substr(pos1+1).c_str());
            }
        }
        if (lines.size() == 0) {
            perror("request header is empty");
            exit(1);
        }
        firstLine = lines[0]; // GET / HTTP/1.1
        std::stringstream ss2(firstLine);
        std::getline(ss2, method, ' ');
        std::getline(ss2, url, ' ');
        std::getline(ss2, version, '\r');
        for (int i = 1; i < lines.size(); i++) {
            if (lines[i].find_first_of(":") == std::string::npos) {
                continue;
            }
            std::string header = lines[i];
            std::stringstream ss3(header);
            std::string key;
            std::string value;
            std::getline(ss3, key, ':');
            ss3.get(); // skip space
            std::getline(ss3, value, '\r');
            if (!key.empty()) {
                headers[key] = value;
            }
        }
        if(headers.count("Host")){
            std::string hostLine = headers["Host"];
            int pos = hostLine.find_first_of(":");
            if(pos!= std::string::npos){
                host = hostLine.substr(0,pos);
                port = hostLine.substr(pos+1);
            }
            else{
                host = hostLine;
                if(method=="POST"||method == "GET"){
                    port = "80";
                }
                if(method=="CONNECT"){
                    port = "443";
                }
            }
        }
    }
}

std::string request::get(std::string key){
    if(headers.count(key)){
        return headers[key];
    }
    else{
        return "";
    }
}

std::string request::to_string() const {
    std::string plainText = method +" "+ url +" "+version + "\r\n";
    for(auto & it:headers){
        plainText += it.first +": " + it.second + "\r\n";
    }
    plainText += "\r\n";
    plainText += body;
    return plainText;
}
std::string request::getMethod() const {
    return method;
}
std::string request::getUrl() const {
    return url;
}
std::string request::getHost() const {
    return host;
}
std::string request::getVersion() const {
    return version;
}
std::string request::getPort() const {
    return port;
}
std::size_t request::getId() const {
    return id;
}
std::string request::getFirstLine() const {
    return firstLine.substr(0, firstLine.find_first_of("\r"));
}

std::string request::getClientIP() const {
    return clientIP;
}
std::string request::getBody() const {
    return body;
}

void request::put(std::string key, std::string val){
    headers[key] = val;
}

void request::erase(std::string key){
    if(headers.count(key)){
        headers.erase(key);
    }
}
// struct tm request::getReqTime() const {
//     return request_t;
// }

void request::setClientIP(std::string _clientIP) {
    clientIP = _clientIP;
}
void request::print() {
    // std::cout << "Method: " << method << std::endl;
    // std::cout << "Url: " << url << std::endl;
    // std::cout << "Version: " << version << std::endl;
    // std::cout << "Port: " << port << std::endl;
    // std::cout << "Host: " << host << std::endl;
    // std::cout << "---------header--------- " << std::endl;
    // for (auto it = headers.begin(); it != headers.end(); it++) {
    //     std::cout << it->first << ": " << it->second << std::endl;
    // }
    // std::cout << "------------------------ " << std::endl;

    std::cout<<"-------request.to_string()(length:"<<to_string().size()<<")--------"<<std::endl;
    std::cout<<to_string()<<std::endl;
    std::cout<<"-----------------------------------"<<std::endl;
}
std::size_t request::next_id = 1;
// bool request::supportCache(){
    
// }