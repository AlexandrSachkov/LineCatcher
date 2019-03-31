#pragma once

#include <vector>

namespace PLP {
    class LineBuffer {
    public:
        LineBuffer(unsigned int size);

        bool set(const char* data, unsigned int size);
        bool append(const char* data, unsigned int size);
        void get(char*& data, unsigned int& size);
        unsigned int getSize();
        void clear();

    private:
        std::vector<char> _data;
        unsigned int _size;
    };
}