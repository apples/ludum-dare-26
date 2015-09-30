#include "meta.hpp"

Inugami::Logger<5> *logger;
Inugami::Profiler *profiler;

GameError::GameError(const std::string& in)
    : err(in)
{}

const char* GameError::what() const noexcept
{
    return ("GameError: "+err).c_str();
}
