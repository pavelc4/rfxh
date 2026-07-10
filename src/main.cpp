#include "text/utf8.hpp"
#include  <iostream>

int main() {
    std::cout << "utf8_char_len('A') = "<< rfxh::text::utf8_char_len('A') << "\n";
    std::cout << "ASCII 'A' (0x41): " << rfxh::text::utf8_char_len(0x41) << "\n";
    std::cout << "2-byte lead (0xC2): " << rfxh::text::utf8_char_len(0xC2) << "\n";
    std::cout << "3-byte lead (0xE2): " << rfxh::text::utf8_char_len(0xE2) << "\n";
    std::cout << "4-byte lead (0xF0): " << rfxh::text::utf8_char_len(0xF0) << "\n";
    return 0;
}
