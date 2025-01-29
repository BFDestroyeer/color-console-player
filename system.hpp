#pragma once

#include <cstdlib>

inline void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#endif _WIN32
#ifdef __unix__
    std::system("clear");
#endif __unix__
}