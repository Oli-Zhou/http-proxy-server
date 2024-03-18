#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <ctime>
#include <mutex>
#include "timer.hpp"
#include "request.h"
#include "response.h"
#include "cache.h"


class logger {
private:
    std::string filePath;
    std::mutex mtx;
    std::ofstream log;
public:
    logger(std::string _filePath) : filePath(_filePath){
        log.open(filePath, std::ios_base::app | std::ios_base::out);
    };
    ~logger() {
        log.close();
    }

    void logMessage(const std::string & message) {
        std::unique_lock<std::mutex> lock(mtx);
        log << message << std::endl;
    }

    void logRequest(const request & request) {
        std::unique_lock<std::mutex> lock(mtx);
        log << request.getId() << ": " << request.getFirstLine() << " from " << request.getClientIP() << " @ " << timer::tmToAsctime(timer::getUTCtime()) << std::endl;
    }

    void loginCacheButExpired(request & request, response & response) {
        std::unique_lock<std::mutex> lock(mtx);
        log << request.getId() << ": " << "in cache, but expired @ " << timer::RFCToAsctime(response.get("Expires")) << std::endl;
    }

    void loginCacheButNeedValidation(request & request) {
        std::unique_lock<std::mutex> lock(mtx);
        log << request.getId() << ": " << "in cache, requires validation" << std::endl;
    }

    void loginCacheAndValid(request & request) {
        std::unique_lock<std::mutex> lock(mtx);
        log << request.getId() << ": " << "in cache, valid" << std::endl;
    }

    void logNotInCache(request & request) {
        std::unique_lock<std::mutex> lock(mtx);
        log << request.getId() << ": " << "not in cache" << std::endl;
    }

    void logRequestContactWithOriginServer(const request & request) {
        std::unique_lock<std::mutex> lock(mtx);
        log << request.getId() << ": " << "Requesting " << request.getFirstLine() << " from " << request.getHost() << std::endl;
    }

    void logResponseContactWithOriginServer(const request & request, const response & response) {
        std::unique_lock<std::mutex> lock(mtx);
        log << request.getId() << ": " << "Received " << response.getFirstLine() << " from " << request.getHost() << std::endl;
    }

    void logCachedButNeedRevalidation(const request & request) {
        std::unique_lock<std::mutex> lock(mtx);
        log << request.getId() << ": " << "cached, but requires re-validation" << std::endl;
    }

    void logCachedButExpired(const request & request, response & response) {
        std::unique_lock<std::mutex> lock(mtx);
        log << request.getId() << ": " << "cached, expires @ " << timer::RFCToAsctime(response.get("Expires")) << std::endl;
    }

    void logNotCacheable(const request & request, const std::string reason) {
        std::unique_lock<std::mutex> lock(mtx);
        log << request.getId() << ": " << "not cacheable because " << reason << std::endl;
    }

    void logRespondingToClient(const request & request, const response & response) {
        std::unique_lock<std::mutex> lock(mtx);
        log << request.getId() << ": " << "Responding " << response.getFirstLine() << std::endl;
    }

    void logTunnelClosed(const request & request) {
        std::unique_lock<std::mutex> lock(mtx);
        log << request.getId() << ": " << "Tunnel closed" << std::endl;
    }
};

#endif
