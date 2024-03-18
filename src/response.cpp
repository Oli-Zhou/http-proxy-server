#include "response.h"

response::response(std::string header,std::string body) {
    all_ctrl(header);
    
    std::stringstream ss(header);
    std::vector<std::string> lines;
    std::string line;
    //process header
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }
    firstLine = lines[0]; // example: HTTP/1.1 200 OK\r\n
    std::stringstream ss2(firstLine);
    std::getline(ss2, version, ' ');
    std::getline(ss2, statusCode, ' ');
    std::getline(ss2, reasonPhrase, '\r');
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
    // process body
    this->body = body;
}


std::string response::get(std::string key) {
    if(headers.count(key)){
        return headers[key];
    }
    else{
        return "";
    }
}

std::string response::to_string() const {
    std::string plainText = version +" "+ statusCode +" "+reasonPhrase + "\r\n";
    for(auto & it:headers){
        plainText += it.first +": " + it.second + "\r\n";
    }
    plainText += "\r\n";
    plainText += body;
    return plainText;
}
std::string response::getBody() const {
    return body;
}
std::string response::getReason() const {
    return reasonPhrase;
}
std::string response::getStatus() const {
    return statusCode;
}
std::string response::getVersion() const {
    return version;
}
std::string response::getFirstLine() const{
    return firstLine.substr(0, firstLine.find_first_of("\r"));
}
bool response::supportCache(){
    std::string cacheControl = get("Cache-Control");
    // std::cout<<"cache control:"<<cacheControl<<std::endl;
    if(cacheControl == ""){
        // msg = "expires";
        return true;
    }
    else if(no_store || priv){
        return false;
    }
    // else if(cacheControl.find("no-store") != std::string::npos||cacheControl.find("private") != std::string::npos){
    //     return false;
    // }
    // else if(cacheControl.find("must-revalidate") != std::string::npos||cacheControl.find("no-cache") != std::string::npos){
    //     // msg = "revalidate";
    //     return false;
    // }
    else{
        return true;
    }
}

// void response::print() {
//     std::cout << "Method: " << method << std::endl;
//     std::cout << "Url: " << url << std::endl;
//     std::cout << "Version: " << version << std::endl;
//     std::cout << "Port: " << port << std::endl;
//     for (auto it = headers.begin(); it != headers.end(); it++) {
//         std::cout << it->first << ": " << it->second << std::endl;
//     }
// }


void response::all_ctrl(std::string buffer){
    std::stringstream ss(buffer);
    std::string line;
    std::tm t;

    while (std::getline(ss, line)){
        if(line.find("Cache-Control") != std::string::npos){
            //save controls
            //Cache-Control: max-age=604800
            int maxage_cnt = 0;
            
            if(line.find("no-store") != std::string::npos){
                //do not cache the response
                no_store = true;
            }
            if(line.find("private") != std::string::npos){
                //do not cache the response
                priv = true;
            }
            if(line.find("no-cache") != std::string::npos){
                //cacheable, revalidation needed
                revalidation_needed = true;
            }
            if(line.find("must-revalidate") != std::string::npos){
                //cacheable, revalidation needed
                revalidation_needed = true;
            }
            //else if(control.find("public") != std::string::npos){}
            if(line.find("max-age") != std::string::npos){
                maxage_cnt ++; //two header fields have diff vals -> invalid ->stale
                if(maxage_cnt != 1){
                    expire_t = response_t;
                }
                max_age = true;
            }
            //end of saving  controls


            
        }
        if(line.find("Date: ") != std::string::npos){
            int pos = line.find(":");
            line = line.substr(pos+2);
            //response_t = stringToTm(line);
        }
        if(line.find("Expires: ") != std::string::npos){
            int pos = line.find(":");
            line = line.substr(pos+2);
            //expire_t = stringToTm(line);
        }
    }

    //std::string control_line;
    //int cache_control = find_ctrl(cacheLines);
    //find_ctrl(cacheLines);

    return;
}



void response::printHeader(){

    std::string plainText = version +" "+ statusCode +" "+reasonPhrase + "\r\n";
    for(auto & it:headers){
        plainText += it.first +": " + it.second + "\r\n";
    }
    plainText += "\r\n";
    std::cout<<"------------response header-------------"<<std::endl;
    std::cout<<plainText<<std::endl;
    std::cout<<"----------------------------------------"<<std::endl;
}
// void response::find_ctrl(std::vector<std::string> cacheLines){
//     int cache_control = 0;
//     int expire_time = 0;
//     //string control_line;
//     for(int i = 0; i < sizeof(cacheLines); i++){
//         int pos = cacheLines[i].find(" ");
//         //Cache-Control: max-age=604800
//         std::string control = cacheLines[i].substr(pos+1);
//         int maxage_cnt = 0;
//         std::cout << "2" <<std::endl;
        
//         if(control.find("no-store") != std::string::npos){
//             //do not cache the response
//             no_store = true;
//             std::cout << "3333" <<std::endl;
//         }
//         else if(control.find("private") != std::string::npos){
//             //do not cache the response
//             priv = true;
//             std::cout << "4444" <<std::endl;
//         }
//         else if(control.find("no-cache") != std::string::npos){
//             //cacheable, revalidation needed
//             revalidation_needed = true;
//            std:: cout << "5555" <<std::endl;
//         }
//         else if(control.find("must-revalidate") != std::string::npos){
//             //cacheable, revalidation needed
//             revalidation_needed = true;
//             std::cout << "6666" <<std::endl;
//         }
//         //else if(control.find("public") != std::string::npos){}
//         if(control.find("max-age") != std::string::npos){
//             maxage_cnt ++; //two header fields have diff vals -> invalid ->stale
//             if(maxage_cnt != 1){
//                 expire_t = response_t;
//                 std::cout << "7777" <<std::endl;
//             }
//             max_age = true;
//         }
//     }
// }
void response::print() {
    std::cout<<"-------response.to_string()(length:"<<to_string().size()<<")--------"<<std::endl;
    std::cout<<to_string()<<std::endl;
    std::cout<<"-----------------------------------"<<std::endl;
}


void response::put(std::string key, std::string val){
    headers[key] = val;
}

struct tm response::getExpireTime() const {
    return expire_t;
}