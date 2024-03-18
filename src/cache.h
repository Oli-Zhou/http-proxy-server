
#ifndef CACHE_H
#define CACHE_H
#include "request.h"
#include "response.h"
#include "timer.hpp"
#include <string.h>
#include <time.h>
#include <utility>
// #include "time_func.h"

/**
 * @brief the result of checking cache when you query the cache to see if the requested object is in the cache.
*/
struct cacheResult {
    bool isCached;
    bool cachedButExpired;
    std::tm expireTime;
    bool cachedButRevalidated;
    bool cachedAndValidated;
};

/**
 * @brief the result of checking the request to see if the requested object is cacheable.
*/
struct cacheInfo {
    bool isCacheable;
    std::string reason;
    std::tm expireTime;
    bool needRevalidation;
};

class cache{
    class LinkedList{
    public:
        std::string key;
        response val;
        LinkedList * next;
        LinkedList * prev;
        LinkedList():key(""),val(response()),next(nullptr),prev(nullptr){
        }
        LinkedList(std::string key,response val):key(key),val(val),next(nullptr),prev(nullptr){
        }
        LinkedList(std::string key,response val,LinkedList * next,LinkedList* prev):key(key),val(val),next(next),prev(prev){
        }
    };
    int capacity;
    int size;
    std::unordered_map<std::string, LinkedList* > responseMap; 
    std::unordered_map<std::string, std::tm > timeMap; 
    LinkedList * head;
    LinkedList * tail;
public:
    cache(int capacity):capacity(capacity),head(new LinkedList()),tail(new LinkedList()),size(0){
        head->next = tail;
        tail->prev = head;
    };
    ~cache(){
        while(head!=nullptr){
            LinkedList * curr = head;
            head = head->next;
            delete curr;
        }
    };
    void remove(LinkedList * node){
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    void addToHead(LinkedList * node){
        node->prev = head;
        node->next = head->next;
        head->next = node;
        node->next->prev = node;
    }

    void moveToHead(LinkedList* node) {
        remove(node);
        addToHead(node);
    }
    LinkedList* removeTail() {
        LinkedList* node = tail->prev;
        remove(node);
        return node;
    }

    // cache():capacity(10){};
    // void cacheit(request request, response response);
    void insert(request request,response response);
    bool hit(request request);
    // bool revalidation(){return false;}
    // void get_from_cache(){}
    // void get_from_server(){}
    bool needRevalidation(request request);
    bool checkExpiration(request request);
    response lookup(request request);
    // void cache_respond(request request);
    // std::tm getReqTime(request request);
    // void insert(request request,response response);
};
        


#endif