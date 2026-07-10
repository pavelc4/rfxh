#pragma once

#include <array>
#include <cstdint>

namespace rfxh {

// Logo limits
inline constexpr int kMaxLogoRows = 200;
inline constexpr int kMaxLogoCols = 50;
inline constexpr int kMaxShading  = 64;

// Render limits
inline constexpr int kFrameWidth  = 80;
inline constexpr int kFrameHeight = 60;
inline constexpr int kMaxPoints   = 80000;
inline constexpr int kGap         = 2;
inline constexpr float kK2        = 120.0f;

// A single UTF-8 codepoint (up to 4 bytes + null)
using Codepoint = std::array<char, 5>;

// A line buffer for logo raw data
using LineBuf = std::array<char, 512>;

// Render cell buffers (FRAME_HEIGHT x FRAME_WIDTH)
using ShadeBuf  = std::array<std::array<int8_t, kFrameWidth>, kFrameHeight>;
using ZBuf      = std::array<std::array<float, kFrameWidth>, kFrameHeight>;
using ColorBuf  = std::array<std::array<int, kFrameWidth>, kFrameHeight>;

} // namespace rfxh
