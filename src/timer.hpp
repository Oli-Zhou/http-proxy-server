#ifndef TIMER_H
#define TIMER_H
#include <iostream>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>

class timer {
public:
    timer() = default;
    ~timer() = default;
    static std::tm getUTCtime() {
        std::time_t t = std::time(nullptr);
        std::tm utc_time;
        gmtime_r(&t, &utc_time);
        return utc_time;
    }
    static std::string tmToAsctime(std::tm time) {
        std::string str(32, '\0');
        asctime_r(&time, &str[0]);
        str.resize(str.find('\0') - 1);
        return str;
    }
    static std::tm RFCToTm(std::string timeStr) {
        std::tm tm = {};
        std::istringstream ss(timeStr);
        // Parse the string into tm struct
        ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
        if (ss.fail()) {
            // std::cout<<"time:'"<<timeStr<<"'"<<std::endl;
            throw std::runtime_error("Failed to parse time string");
        }
        return tm;
    }
    static bool isEarlier(std::tm time1, std::tm time2) {
        return std::difftime(std::mktime(&time1), std::mktime(&time2)) < 0;
    }

    static std::string RFCToAsctime(std::string timeStr) {
        if(timeStr==""){
            return "";
        }
        std::tm tm = RFCToTm(timeStr);
        return tmToAsctime(tm);
    }
};

#endif