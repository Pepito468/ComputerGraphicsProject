// Library for debugging
#ifndef ENGINE_DEBUG_H
#define ENGINE_DEBUG_H

#include <stdexcept>
#include <string>
#include <iostream>

/* Prints a warning to stdout */
void warning(std::string string) {
    std::cout << "WARNING: " << string << std::endl;
}

/* Prints and error to console and then throws the error itself */
void error(std::string string) {
    std::cout << "ERROR: " << string << std::endl;
    throw std::runtime_error(string);
}

#endif
