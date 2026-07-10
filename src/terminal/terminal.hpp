#pragma once

namespace rfxh::terminal {
    class RawModeGuard {
        public:
            RawModeGuard();
            ~RawModeGuard();

            RawModeGuard (const RawModeGuard&) = delete;
            RawModeGuard& operator=(const RawModeGuard&) = delete;
private:
    bool action_ = false;
    };
}
