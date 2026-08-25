// Library for debugging
#ifndef ENGINE_DEBUG_H
#define ENGINE_DEBUG_H

#include <format>
#include <stdexcept>
#include <string>
#include <iostream>
#include <set>

#define COLOR_DEFAULT "\033[0m"
#define COLOR_LIGHT_BLUE "\033[94m"
#define COLOR_YELLOW "\033[93m"
#define COLOR_RED "\033[91m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_BRIGHT_GREEN "\033[92m"

/// Prints a string to stdout
inline void println(const std::string& string)
{
    std::cout << string << std::endl;
}

/// Prints info to stdout. Should be used to print info about the system while it is running
inline void info(const std::string& string) {
    std::cout << COLOR_LIGHT_BLUE << "[INFO] " << COLOR_DEFAULT << string << std::endl;
}

/// Prints a log to stdout
inline void log(const std::string& string)
{
    std::cout << COLOR_MAGENTA << "[LOG] " << COLOR_DEFAULT << string << std::endl;
}

/// Prints a warning to stdout
inline void warning(const std::string& string) {
    std::cout << COLOR_YELLOW << "[WARNING] " << COLOR_DEFAULT << string << std::endl;
}

/// Prints an error to console and then throws the error itself
inline void error(const std::string& string) {
    std::cout << COLOR_RED << "[ERROR] " << COLOR_DEFAULT << string << std::endl;
    throw std::runtime_error(string);
}

/// If the given expression isn't true, calls error with the given message
inline void _assert(const bool expression, const std::string& message) {
    if (!expression) {
        error(message);
    }
}

template <typename T>
std::string setToString (const std::set<T>& s)
{
    if (s.empty()) return "[]";

    std::string res = "[";
    for (const T& i : s)
    {
        res.append(std::format("{}, ", i));
    }
    res.pop_back();
    res.pop_back();
    res.append("]");
    return res;
}
template <typename T>
struct std::formatter<std::set<T>> : std::formatter<std::string> {
    auto format(std::set<T> s, format_context& ctx) const {
        return formatter<string>::format(setToString(s), ctx);
    }
};
#endif
