#ifndef REQUEST_H
#define REQUEST_H

#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<sstream>

// #include "time_func.h"

class request {
private:
    std::size_t id;
    static std::size_t next_id;
    std::string method;
    std::string url;
    std::string version;
    std::string port;
    std::string host; 
    std::unordered_map<std::string, std::string> headers;
    std::string firstLine;
    std::string clientIP;
    // struct tm request_t;
    std::string body;
public:
    
    int min_fresh;
    int max_stale;
    
    request(char* httpRequestClearText,int requestLen);
    void parseRequest(char* httpRequestPlainText,int requestLen);
    std::string to_string() const;
    std::string get(std::string key);
    std::string getMethod() const;
    std::string getUrl() const;
    std::string getHost() const;
    std::string getVersion() const;
    std::string getPort() const;
    std::size_t getId() const;
    std::string getFirstLine() const;
    std::string getClientIP() const;
    // struct tm getReqTime() const;
    std::string getBody() const;
    void put(std::string key, std::string val);
    void setClientIP(std::string _clientIP);
    void print() ;
    // bool supportCache();
    void erase(std::string key);
};

#endif