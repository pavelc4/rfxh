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

    static int skip_ansi(const char *p) {
      if (p[0] != '\033' || p[1] != '[')
        return 0;
      int i = 2;
      while (p[i] && ((p[i] >= '0' && p[i] <= '9') || p[i] == ';'))
        i++;
      if (p[i])
        i++;
      return i;
    }
}
