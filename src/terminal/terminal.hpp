#pragma once

namespace rfxh::terminal {

    class RawModeGuard {
    public:
        RawModeGuard();
        ~RawModeGuard();

        void restore();

        RawModeGuard(const RawModeGuard&) = delete;
        RawModeGuard& operator=(const RawModeGuard&) = delete;

    private:
        bool active_ = false;
    };

    void install_signal_handlers();
    bool consume_resize_flag();
    int get_term_rows();

} // namespace rfxh::terminal
