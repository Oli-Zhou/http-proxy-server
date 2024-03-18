#ifndef RESPONSE_H
#define RESPONSE_H

#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<sstream>
#include"timer.hpp"

class response {
private:
    std::string version;
    std::string statusCode;
    std::string reasonPhrase;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    std::string firstLine;
public:
    std::string content;
    struct tm response_t;
    struct tm expire_t;
    bool no_store;
    bool priv;
    bool max_age;
    bool revalidation_needed;

    void put(std::string key, std::string val);
    std::string get(std::string key);
    void all_ctrl(std::string buffer);
    //void find_ctrl(std::vector<std::string> cacheLines);
    
    std::string to_string() const;
    response(){no_store=false; priv=false; max_age=false; revalidation_needed=false;};
    response(std::string header,std::string body);
    response(std::string _firstLine) : firstLine(_firstLine) {}
    std::string getReason() const;
    std::string getStatus() const;
    std::string getVersion() const;
    std::string getBody() const;
    std::string getFirstLine() const;
    struct tm getExpireTime() const;
    void print() ;
    void printHeader() ;
    // bool supportCache(std::string & msg);
    bool supportCache();
};

#endif
