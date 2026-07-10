#include "terminal/terminal.hpp"
#include <iostream>

int main() {
    rfxh::terminal::RawModeGuard raw_mode;
    std::cout << "raw mode active, cursor hidden\n";
    return 0;
}
