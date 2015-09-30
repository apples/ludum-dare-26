#ifndef LOG_H
#define LOG_H

#include "inugami/logger.hpp"
#include "inugami/profiler.hpp"

#include <string>

extern Inugami::Logger<5> *logger;
extern Inugami::Profiler *profiler;

class GameError : public std::exception
{
public:
    GameError(const std::string& in);
    virtual const char* what() const noexcept override;

    std::string err;
};

#endif // LOG_H
