#ifndef headerfilelogger
#define headerfilelogger
#include <iostream>
#include <mutex>
#include <ctime>

enum Status {INFO, ERROR, WARN, IMPO};

struct Logger_t{
    // Green Red Yellow End
    std::string ColorMask[4] = {"\033[32m", "\033[31m", "\033[33m", "\033[31m"};
    std::string END = "\033[m";
    mutable char buffer[1024];

    std::ostream & operator()(std::ostream & os, Status type, std::string model) const {

        os << ColorMask[type];

        if(type == INFO)  os << "INFO ";
        if(type == ERROR) os << "ERRO ";
        if(type == WARN)  os << "WARN ";
        if(type == IMPO)  os << "IMPO ";

        os << END;

        time_t now = time(0);
        tm *ltm = localtime(&now);

        sprintf(buffer, "[%02d-%02d|%02d:%02d:%02d]", 1 + ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

        os << buffer;

        os << " [" << model << "] ";

        for(int i = 1; i < 20 - static_cast<int>(model.size()); i++) os << ' ';

        return os;
    }
} Logger;


#endif