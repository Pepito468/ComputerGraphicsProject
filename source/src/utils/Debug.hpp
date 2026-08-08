// Library for debugging
#ifndef ENGINE_DEBUG_H
#define ENGINE_DEBUG_H

#include <format>
#include <glm/glm.hpp>
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

/// If the given expression isn't true, calls error with the given message
inline void _assert(const bool expression, const std::string& message) {
    if (!expression) {
        error(message);
    }
}

/// Specialization to make vec3 compatible with std::format
template <>
struct std::formatter<glm::vec3> : std::formatter<std::string> {
    auto format(glm::vec3 p, format_context& ctx) const {
        return formatter<string>::format(
          std::format("[{}, {}, {}]", p.x, p.y, p.z), ctx);
    }
};
#endif
