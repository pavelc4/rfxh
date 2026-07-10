#include "terminal/terminal.hpp"
#include <iostream>
#include <unistd.h>

int main() {
    rfxh::terminal::install_signal_handlers();
    rfxh::terminal::RawModeGuard raw_mode;

    std::cout << "press ctrl+c to test signal cleanup, or resize terminal\n";
    for (int i = 0; i < 20; i++) {
        if (rfxh::terminal::consume_resize_flag())
            std::cout << "resize detected!\n";
        sleep(1);
    }
    return 0;
}
