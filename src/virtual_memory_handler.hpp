#pragma once
#include "config.hpp"
#include <fstream>
#include <algorithm>

void save_memory(const string& path, std::array<byte, 0x10000>& memory) {
    std::ofstream file(path, std::ios::binary);

    file.write(reinterpret_cast<char *>(memory.data()), 0x10000);

    file.close();
}

void load_memory(const string& path, std::array<byte, 0x10000>& memory) {
    std::fill(memory.begin(), memory.end(), 0);

    std::ifstream file(path, std::ios::binary);

    file.read(reinterpret_cast<char *>(memory.data()), 0x10000);

    file.close();
}

