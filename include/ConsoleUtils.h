#ifndef CONSOLE_UTILS_H
#define CONSOLE_UTILS_H

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

inline void timeDelay(int milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

inline void clearScreen()
{
    std::cout << "\033[2J\033[3J\033[1;1H" << std::flush;
}

inline void printMenuHeader(const std::string &title)
{
    const std::string border(title.size() + 4, '=');
    std::cout << "\n" << border << "\n"
              << "  " << title << "\n"
              << border << "\n";
}

inline void printMenuSeparator()
{
    std::cout << "  --------------------\n";
}

inline void waitForContinue()
{
    std::cout << "\nPress Enter to continue..." << std::flush;
    std::string input;
    std::getline(std::cin, input);
}

#endif
