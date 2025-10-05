#pragma once

#include <iostream>
#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <raylib.h>

//screen dimensions
const int COLS = 32;
const int ROWS = 24;

typedef uint8_t byte;
typedef uint16_t word;
typedef bool bit;
typedef std::string string;
typedef std::array<string, ROWS> screen_buffer;

void print_hex(byte number) {
    std::cout << "0x" << std::hex << std::uppercase << static_cast<int>(number) << std::endl;
}
void print_hex(word number) {
    std::cout << "0x" << std::hex << std::uppercase << static_cast<int>(number) << std::endl;
}
void print_hex(bool number) {
    std::cout << "0x" << std::hex << std::uppercase << static_cast<int>(number) << std::endl;
}

word concat_hex(byte a, byte b) {
    return (static_cast<word>(a) << 8) | b;
}

std::array<byte, 2> deconcat_hex(word val) {
    return {static_cast<byte>(val >> 8), static_cast<byte>(val)};
}