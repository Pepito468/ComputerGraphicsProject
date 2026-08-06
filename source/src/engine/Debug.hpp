// Library for debugging
#ifndef ENGINE_DEBUG_H
#define ENGINE_DEBUG_H

#include <stdexcept>
#include <string>
#include <iostream>

/// Prints a string to stdout
inline void println(const std::string& string)
{
    std::cout << string << std::endl;
}

/// Prints a log to stdout
inline void log(const std::string& string)
{
    std::cout << "LOG: " << string << std::endl;
}

/// Prints a warning to stdout
inline void warning(const std::string& string) {
    std::cout << "WARNING: " << string << std::endl;
}

/// Prints an error to console and then throws the error itself
inline void error(const std::string& string) {
    std::cout << "ERROR: " << string << std::endl;
    throw std::runtime_error(string);
}
#endif
