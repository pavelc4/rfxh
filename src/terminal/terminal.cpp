#include "terminal/terminal.hpp"
#include "platform/terminal.hpp"

namespace rfxh::terminal {

    RawModeGuard::RawModeGuard() {
        platform::terminal_init();
        platform::cursor_hide();
        active_ = true;
    }

    RawModeGuard::~RawModeGuard() {
        restore();
    }

    void RawModeGuard::restore() {
        if (active_) {
            platform::terminal_restore();
            active_ = false;
        }
        platform::cursor_show();
    }

    void install_signal_handlers() {
        // Signal handlers are installed in platform::terminal_init()
    }

    int get_term_rows() {
        return platform::terminal_rows();
    }

    bool consume_resize_flag() {
        return platform::consume_resize();
    }

} // namespace rfxh::terminal
