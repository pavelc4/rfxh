#pragma once

namespace rfxh::text {

    static int utf8_char_len(unsigned char c) {
      if (c < 0x80)
        return 1;
      if ((c & 0xE0) == 0xC0)
        return 2;
      if ((c & 0xF0) == 0xE0)
        return 3;
      if ((c & 0xF8) == 0xF0)
        return 4;
      return 1;
    }
}
