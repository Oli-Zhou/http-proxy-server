#include "cache.h"
// void cache::cacheit(request request, response response){
//     if(response.priv){
//         std::cout << "not cacheable: private" << std::endl;
//     }
//     else if(response.no_store){
//         std::cout << "not cacheable: no-store" << std::endl;
//     }
//     else{
//         for(auto it = cache.begin(); it != cache.end(); it++){
//             if(strcmp((request.getFirstLine()).c_str(), (it->first).getFirstLine().c_str())==0){
//                 response = it->second;
//                 break;
//             }
//         }
        
//         cache[request] = response;
//         if(response.revalidation_needed){
//             std::cout << "cached, but requires re-validation" << std::endl;
//         }
//         else if(response.max_age){
//             std::cout << " cached, expires at " << asctime(&(response.expire_t)) << std::endl;
//         }
//     }
// }
    
// std:: cache::getReqTime(request request){
//     return timeMap[request.getFirstLine()];
// }
response cache::lookup(request request){
    std::string key = request.getFirstLine();
    LinkedList* node = responseMap[key];
    moveToHead(node);
    // std::cout<<"--------cache-lookup---------------"<<std::endl;
    // std::cout<<"request:"<<request.getFirstLine()<<std::endl;
    // std::cout<<"response:"<<node->val.getFirstLine()<<std::endl;
    // std::cout<<"-----------------------------------"<<std::endl;
    return node->val;
}
// bool cache::revalidation(){return false;}
bool cache::hit(request request){
    std::string firstLine = request.getFirstLine();
    // std::cout<<"--------cache-hit---------------"<<std::endl;
    // std::cout<<"request:"<<request.getFirstLine()<<std::endl;
    // std::cout<<"hit?:"<<responseMap.count(firstLine)<<std::endl;
    // std::cout<<"---------------------------------"<<std::endl;
    return responseMap.count(firstLine);
}
// void cache::get_from_cache(){}
// void cache::get_from_server(){}
bool cache::needRevalidation(request request){
    response cachedResponse = lookup(request);
    std::string key = request.getFirstLine();
    std::string contentControl = cachedResponse.get("Content-Control");
    std::string eTag = cachedResponse.get("ETag");
    std::string lastModified = cachedResponse.get("Last_Modified");
    //int mustRevalidationPos = contentControl.find("must-revalidate");
    //int no_cache = contentControl.find("no-cache");
    if(eTag!=""||lastModified!=""||cachedResponse.revalidation_needed){
        return true;
    }
    return false;
    
}

bool cache::checkExpiration(request request){
    response cachedResponse = lookup(request);
    std::string expires = cachedResponse.get("Expires");
    std::string cacheControl = cachedResponse.get("Cache-Control");
    std::string lastModified = cachedResponse.get("Last-Modified");
    std::string key = request.getFirstLine();
    //check whether exceed max cache age.
    if(lastModified!=""){
        // std::cout<<"lastModified:"<<lastModified<<std::endl;
        std::tm lastModifiedTime= timer::RFCToTm(lastModified);
        if(timer::isEarlier(lastModifiedTime,timer::getUTCtime())){
            return true;
        }
    }
    else if(expires!=""){
        // std::cout<<"expireTime:"<<expires<<std::endl;
        std::tm expireTime = timer::RFCToTm(expires);
        //min-fresh max-stale
         std::string request_cacheControl = request.get("Cache-Control");
         if(request_cacheControl!=""){
             //valid time = max-age + max-stale - min-fresh
             //if (now time - expire time < max_stale - min_fresh) then valid
             std::tm currTime = timer::getUTCtime();
             if(std::difftime(std::mktime(&currTime), std::mktime(&expireTime)) > (request.max_stale-request.min_fresh)){
                return true; // not valid
            }
         }
        //no rewuest_cacheControl
        if(timer::isEarlier(expireTime,timer::getUTCtime())){
            return true;
        }
    }
    else if (cacheControl!=""){
        int pos = cacheControl.find("max-age=");
        if(cachedResponse.max_age){
            int maxAge = std::stoi(cacheControl.substr(pos+8));
            //min-fresh max-stale
            std::string request_cacheControl = request.get("Cache-Control");
            if(request_cacheControl!=""){
                //valid time = max-age + max-stale - min-fresh
                //if (expire time - now time > min_fresh - max_stale) then valid
                maxAge += request.max_stale - request.min_fresh;
         }

            std::tm cacheValidTime = timeMap[key];
            std::tm currTime = timer::getUTCtime();
            if(std::difftime(std::mktime(&currTime), std::mktime(&cacheValidTime)) > maxAge){
                return true;
            }
        }
    }
    return false;
}

void cache::insert(request request,response response){
    std::string key = request.getFirstLine();
    timeMap[key] = timer::getUTCtime();
    if (!responseMap.count(key)) {
        LinkedList* node = new LinkedList(key, response);
        responseMap[key] = node;
        addToHead(node);
        ++size;
        if (size > capacity) {
            LinkedList* removed = removeTail();
            responseMap.erase(removed->key);
            timeMap.erase(removed->key);
            delete removed;
            --size;
        }
    }
    else {
        LinkedList* node = responseMap[key];
        node->val = response;
        moveToHead(node);
    }
    // std::cout<<"--------cache-insert(size:"<<size<<")-------"<<std::endl;
    // std::cout<<"request:"<<key<<std::endl;
    // std::cout<<"response:"<<response.getFirstLine()<<std::endl;
    // std::cout<<"--------------------------------------------"<<std::endl;
}

// void cache::cache_respond(request request){
//     response response;
//     bool found_in_cache = false;
//     for(auto it = cache.begin(); it != cache.end(); it++){
//         if(strcmp(request.getFirstLine().c_str(), (it->first).getFirstLine().c_str())==0){
//             response = it->second;
//             found_in_cache = true;
//             break;
//         }
//     }
        
//     if(!found_in_cache){
//         std::cout << "not in cache" << std::endl;
//     }
//     else if(response.revalidation_needed){
//         std::cout << " in cache, requires validation" << std::endl;
//         if(revalidation()){ //revalidation accepted
//             get_from_cache();
//         }
//         else{ //revalidation failed
//             get_from_server();
//         }
//     }
//     else if(response.max_age){
//         if(checkexpiration(request.getReqTime(), response.expire_t)){
//             std::cout << "in cache, but expired at " << asctime(&(response.expire_t)) << std::endl;
//             get_from_server();
//         }
//         else{
//             get_from_cache();
//         }
//     }
//     else{
//         std::cout << "in cache, valid" << std::endl;
//         get_from_cache();
//     }

// }
        

